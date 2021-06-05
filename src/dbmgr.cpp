
#include "dbmgr.h"

#include "moby.h"
#include "dbmgr_internal.h"
#include "targets.h"

#include <stdio.h> //printf, fprintf
#include <stdlib.h> //malloc, free
#include <stddef.h> //size_t

DummyTarget dummy;

static uint8_t* workBuf = NULL;

void DBMgr::refreshStateAndDB(void) {
	if (!this->targetsState[this->currentTarget].isAttached && !this->targetsState[this->currentTarget].hasValidState) {
		resetTargetState(&this->targetsState[this->currentTarget]);
		this->mobysCount = 0;
		return;
	}
	switch (this->currentTarget) {
	case TARGET_PLATFORM_PS2EMU:
	{
		uint8_t* eem = (uint8_t*)malloc(EE_MEMORY_SIZE);
		if (eem == NULL) {
			fprintf(stderr, "[DBMgr] Could not allocate eeMemory buffer for PS2Emu target.\n");
			fprintf(stderr, "%s failed !\n", __func__);
			return;
		}
		else {
			TargetState* t = &this->targetsState[TARGET_PLATFORM_PS2EMU];
			size_t rs = t->target->readTargetMemory(0, eem, EE_MEMORY_SIZE);
			if (rs != EE_MEMORY_SIZE) {
				fprintf(stderr, "[DBMgr] %d bytes read from EE memory, instead of %d !\n", rs, EE_MEMORY_SIZE);
				free(eem);
				return;
			}
			int ret = DBMgrInternal::getMobyStackAddressesAndGameForPS2Emu(eem, &t->currentGame, &t->mobyStackBase, &t->mobyStackMax);
			if (ret == DBMGR_INTERNAL_OK) {
				printf("[DBMgr] getMobyStackAddressesAndGameForPS2Emu returned OK!\n");
				t->hasValidState = true;
				this->refresh();
			}
			else {
				fprintf(stderr, "[DBMgr] getMobyStackAddressesAndGameForPS2Emu failed !\n");
			}
			free(eem);
			return;
		}
	}

	case TARGET_PLATFORM_PS3:
	case TARGET_PLATFORM_PS3EMU:
	case TARGET_PLATFORM_PSVITA:
	default:
		return;
	}
}

void DBMgr::refresh(void) {
	if (!this->targetsState[this->currentTarget].isAttached && !this->targetsState[this->currentTarget].hasValidState) {
		resetTargetState(&this->targetsState[this->currentTarget]);
		this->mobysCount = 0;
		return;
	}

	if (workBuf == NULL) {
		workBuf = (uint8_t*)malloc(MAX_MOBYS_COUNT * MOBY_SIZE);
		if (workBuf == NULL) {
			fprintf(stderr, "[DBMgr] Couldn't allocate work buffer !\n");
			return;
		}
	}
	MobyEntry* operatingTable = (this->activeMobysBank == this->mobysData1) ? this->mobysData2 : this->mobysData1;

	//Maybe the for loop could be made common... for now, this works.
	switch (this->currentTarget) {
	case TARGET_PLATFORM_PS2EMU:
	{
		//Note : this routine is highly optimized for ReadProcessMemory.
		TargetState* t = &this->targetsState[TARGET_PLATFORM_PS2EMU];
		this->mobysCount = (t->mobyStackMax - t->mobyStackBase) / MOBY_SIZE; //First estimation, this is the biggest it can get.

		size_t expectedSize = this->mobysCount * MOBY_SIZE;
		size_t rsiz = t->target->readTargetMemory(t->mobyStackBase, workBuf, expectedSize);
		if (rsiz != expectedSize) {
			fprintf(stderr, "[DBMgr] Failed refresh, because I could only read %d byte(s) instead of expected %d while getting Moby data from PS2Emu.\n", rsiz, expectedSize);
			return;
		}

		size_t aliveMobysCount = 0;
		for (size_t i = 0; i < this->mobysCount; i++) {
			uint8_t* mobyBuf = &workBuf[i * MOBY_SIZE];
			uint8_t state = mobyBuf[0x20];
			uint16_t uid = *(uint16_t*)((uintptr_t)mobyBuf + 0xB2);
			if (state >= 0xFD) { //The game considers a moby slot as free if this checks succeeds (it also checks to framecounter-dependent thing, but let's pretend it doesn't)
				continue; //Ignore moby
			}
			MobyTool::LEBufferToMoby(mobyBuf, &operatingTable[aliveMobysCount].moby);
			operatingTable[aliveMobysCount].mobyAddr = (uintptr_t)(t->mobyStackBase + (aliveMobysCount * MOBY_SIZE));
			aliveMobysCount++;
		}
		this->mobysCount = aliveMobysCount;
		break;
	}

	//TODO : code behaviour for those platforms
	case TARGET_PLATFORM_PS3:
	case TARGET_PLATFORM_PS3EMU:
	case TARGET_PLATFORM_PSVITA:
	default:
		return;
	}
	this->activeMobysBank = operatingTable;
}