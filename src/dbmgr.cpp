
#include "dbmgr.h"

#include "moby.h"
#include "dbmgr_internal.h"
#include "targets.h"

#include <stdio.h>
#include <stdlib.h>

DummyTarget dummy;

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
	uint8_t mobyBuf[MOBY_SIZE];
	Moby moby;

	Moby* operatingTable = (this->activeMobysBank == this->mobysData1) ? this->mobysData2 : this->mobysData1;

	//Maybe the for loop could be made common for PS2 and Vita, but heh, works for now.
	switch (this->currentTarget) {
	case TARGET_PLATFORM_PS2EMU:
	{
		TargetState* t = &this->targetsState[TARGET_PLATFORM_PS2EMU];
		this->mobysCount = (t->mobyStackMax - t->mobyStackBase) / MOBY_SIZE; //First estimation, this is the biggest it can get.

		for (int i = 0; i < this->mobysCount; i++) {
			size_t rsize = t->target->readTargetMemory(t->mobyStackBase + (i * MOBY_SIZE), mobyBuf, MOBY_SIZE);
			if (rsize != MOBY_SIZE) {
				fprintf(stderr, "[DBMgr] Could only read %d byte(s) instead of expected %d while getting Moby data from PS2Emu.\n Moby %d will be skipped.\n", rsize, MOBY_SIZE, i);
				continue;
			}
			MobyTool::LEBufferToMoby(mobyBuf, &operatingTable[i]);
			if (operatingTable[i].state > 0xFD) { //Check borrowed from the game, if this condition is met then we *should* have reached table end.
				/*Game also checks that operatingTable[i].collisionCounter <= TIME, but too lazy to find TIME reliably*/
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