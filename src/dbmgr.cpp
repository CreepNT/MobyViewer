
#include "dbmgr.h"

#include "moby.h"
#include "dbmgr_internal.h"
#include "targets.h"

#include <stdio.h> //printf, fprintf
#include <stdlib.h> //malloc, free
#include <stddef.h> //size_t

DummyTarget dummy;

bool DBMgr::refreshStateAndDB(void) {
	if (!this->targetsState[this->currentTarget].isAttached && !this->targetsState[this->currentTarget].hasValidState) {
		resetTargetState(&this->targetsState[this->currentTarget]);
		this->mobysCount = 0;
		return true;
	}
	switch (this->currentTarget) {
	case TARGET_PLATFORM_PS2EMU:
	{
		std::unique_ptr<uint8_t[]> eem_p;
		try {
			eem_p = std::make_unique<uint8_t[]>(EE_MEMORY_SIZE);
		}
		catch (std::bad_alloc&) {
			fprintf(stderr, "[DBMgr] Could not allocate eeMemory buffer for PS2Emu target.\n");
			fprintf(stderr, "%s failed !\n", __func__);
			return false;
		}
		uint8_t* eem = eem_p.get();
		TargetState* t = &this->targetsState[TARGET_PLATFORM_PS2EMU];
		size_t rs = t->target->readTargetMemory(0, eem, EE_MEMORY_SIZE);
		if (rs != EE_MEMORY_SIZE) {
			fprintf(stderr, "[DBMgr] %zu bytes read from EE memory, instead of %u !\n", rs, EE_MEMORY_SIZE);
			return false;
		}
		int ret = DBMgrInternal::getMobyStackAddressesAndGameForPS2Emu(eem, &t->currentGame, &t->mobyStackBase, &t->mobyStackMax);
		if (ret == DBMGR_INTERNAL_OK) {
			printf("[DBMgr] getMobyStackAddressesAndGameForPS2Emu returned OK!\n");
			t->hasValidState = true;
			return this->refresh();
		}
		else {
			fprintf(stderr, "[DBMgr] getMobyStackAddressesAndGameForPS2Emu failed !\n");
			return false;
		}
	}

	case TARGET_PLATFORM_PS3:
	case TARGET_PLATFORM_PS3EMU:
	case TARGET_PLATFORM_PSVITA:
	default:
		return false;
	}
}

bool DBMgr::refresh(void) {
	static uint8_t* workBuf = NULL;
	if (!this->targetsState[this->currentTarget].isAttached && !this->targetsState[this->currentTarget].hasValidState) {
		resetTargetState(&this->targetsState[this->currentTarget]);
		this->mobysCount = 0;
		return true;
	}

	if (workBuf == NULL) {
		workBuf = (uint8_t*)malloc(MAX_MOBYS_COUNT * MOBY_SIZE);
		if (workBuf == NULL) {
			fprintf(stderr, "[DBMgr] Couldn't allocate work buffer !\n");
			return false;
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
			fprintf(stderr, "[DBMgr] Failed refresh, because I could only read %zu byte(s) instead of expected %zu while getting Moby data from PS2Emu.\n", rsiz, expectedSize);
			return false;
		}

		unsigned int aliveMobysCount = 0;
		for (size_t i = 0; i < this->mobysCount; i++) {
			uint8_t* mobyBuf = &workBuf[i * MOBY_SIZE];
			uint8_t state = mobyBuf[0x20];
			if (state >= 0xFD) { //The game considers a moby slot as free if this checks succeeds (it also checks to framecounter-dependent thing, but let's pretend it doesn't)
				continue; //Ignore moby
			}
			if (t->currentGame == GAME_RC4) MobyTool::LEBufferToRC4Moby(mobyBuf, &operatingTable[aliveMobysCount].moby.RC4moby);
			else MobyTool::LEBufferToOG3Moby(mobyBuf, &operatingTable[aliveMobysCount].moby.OG3moby);
			
			operatingTable[aliveMobysCount].mobyAddr = (uintptr_t)(t->mobyStackBase + (i * MOBY_SIZE));
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
		return false;
	}
	this->activeMobysBank = operatingTable;
	return true;
}