
#include "dbmgr.h"

#include "moby.h"
#include "dbmgr_internal.h"
#include "targets.h"

#include <stdio.h>
#include <stdlib.h>

DummyTarget dummy;

void DBMgr::refreshStateAndDB(void) {
	if (!this->targetsState[this->currentTarget].isAttached) {
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
			uint32_t stckBasePtr = 0, stckTopPtr = 0;
			DBMgrInternal::getMobyStackAddressesAndGameForPS2Emu(eem, &t->currentGame, &stckBasePtr, &stckTopPtr);
			t->target->readTargetMemory(stckBasePtr, (uint8_t*)&t->mobyStackBase, sizeof(t->mobyStackBase));
			t->target->readTargetMemory(stckTopPtr, (uint8_t*)&t->mobyStackTop, sizeof(t->mobyStackTop));
			this->refresh();
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
	if (!this->targetsState[this->currentTarget].isAttached) {
		this->mobysCount = 0;
		return;
	}
	uint8_t mobyBuf[MOBY_SIZE];
	Moby moby;

	Moby* operatingTable = (this->activeMobysBank == this->mobysData1) ? this->mobysData2 : this->mobysData1;

	//Maybe the for loop could be made common for PS2 and Vita, but heh, works for now.
	switch (this->currentTarget) {
	case TARGET_PLATFORM_PS2EMU:
	{
		TargetState* t = &this->targetsState[TARGET_PLATFORM_PS2EMU];
		this->mobysCount = (t->mobyStackTop - t->mobyStackBase) / MOBY_SIZE;

		for (int i = 0; i < this->mobysCount; i++) {
			size_t rsize = t->target->readTargetMemory(t->mobyStackBase + (i * MOBY_SIZE), mobyBuf, MOBY_SIZE);
			if (rsize != MOBY_SIZE) {
				fprintf(stderr, "[DBMgr] Could only read %d byte(s) instead of expected %d while getting Moby data from PS2Emu.\n Moby %d will be skipped.\n", rsize, MOBY_SIZE, i);
				continue;
			}
			MobyTool::LEBufferToMoby(mobyBuf, &operatingTable[i]);
			if (operatingTable[i].UID == 0 && i != 0) { //Only Moby #0 can have UID 0, so if we found another, the Moby table ended at last entry.
				this->mobysCount = (i - 1);
				break;
			}
		}
		
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