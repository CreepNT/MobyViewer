
#include <cstdint>
#include <cstring>

#include "moby.h"
#include "widgets.h"
#include "targets.h"
#include "targets/ps2emu.h"

extern DummyTarget dummy;

namespace {
	constexpr uint32_t MAX_MOBYS_COUNT = 0x1000; //Maybe more ?

	typedef struct TargetState {
		Target* target;
		bool isAttached = false;
		bool hasValidState = false; //This is true when stackBase, stackMax are valid, and currentGame != GAME_INVALID
		uint32_t mobyStackBase = 0, mobyStackMax = 0;
		GameID currentGame = GAME_INVALID;
	} TargetState;

	typedef struct MobyEntry {
		union {
			Moby OG3moby;
			RC4Moby RC4moby;
		} moby;
		uintptr_t mobyAddr; //Address in target's address space where the moby is stored
	} MobyEntry;

	//Sets a TargetState back to default. Used for internal cleanup.
	inline void resetTargetState(TargetState* t) {
		t->currentGame = GAME_INVALID;
		t->hasValidState = false;
		t->isAttached = false;
		t->mobyStackBase = 0;
		t->mobyStackMax = 0;
	}
}

class DBMgr {
public:
	//Returns the number of mobys in database - all mobys in database are active mobys
	unsigned int getMobysCount(void) {
		return this->mobysCount;
	}

	//Copies a moby's data to the provided area. m is not written to if entryNum is invalid.
	void getMobyContent(unsigned int entryNum, Moby* m) {
		if (entryNum < mobysCount)
			memcpy(m, &this->activeMobysBank[entryNum], sizeof(Moby));
	}

	//Returns the pointer to selected moby, or nullptr if entryNum is invalid.
	//Cast pointer to RC4Moby* if the current game is RC4, else cast it to Moby*
	void* getMobyPointer(unsigned int entryNum) {
		if (entryNum < mobysCount) {
			if (this->targetsState[currentTarget].currentGame != GAME_RC4)
				return &(this->activeMobysBank[entryNum].moby.OG3moby);
			else
				return &(this->activeMobysBank[entryNum].moby.RC4moby);
		}
		else return nullptr;
	}

	//Returns the address of the selected moby in target's address space, or 0 if entryNum is invalid.
	uintptr_t getMobyAddress(unsigned int entryNum) {
		if (entryNum < mobysCount)
			return this->activeMobysBank[entryNum].mobyAddr;
		else return 0;
	}

	//Refreshes data in the database. Call this to read target's memory again. Returns 0 on failure.
	bool refresh(void);

	//Refreshes data in the database, and all internal state variables used. You need to call this when refreshing for the first time,
	//because DBMgr needs to setup said internals state variables. You can also call this when i.e. switching game/planets on PS2.
	//Warning : this may be SIGNIFICANTLY slower than just a refreshDB(), only call this for specific cases.
	//Returns 0 on failure.
	bool refreshStateAndDB(void);

	//Changes the target platform. Specifying an invalid target will set it to TARGET_PLATFORM_NONE.
	void setNewTarget(int target) {
		if (target >= 0 && target < TARGET_PLATFORMS_COUNT) {
			this->currentTarget = target;
			this->mobysCount = 0; //When switching to a new target, the moby table becomes invalid.
		}
		else this->currentTarget = TARGET_PLATFORM_NONE;
	}

	//Returns the current target
	int getCurrentTarget(void) {
		return this->currentTarget;
	}

	//Calls the target platform's initialize function, except if it is already attached and forceReinit isn't true.
	int initializeTarget(void* targetInitParams = nullptr, bool forceReinit = false) {
		if (this->targetsState[this->currentTarget].isAttached && !forceReinit) {
			return INIT_OK;
		}
		if (forceReinit) {
			this->targetsState[this->currentTarget].target->cleanup();
		}
		int ret = this->targetsState[this->currentTarget].target->init(targetInitParams);
		this->targetsState[this->currentTarget].isAttached = (ret == INIT_OK);
		return ret;
	}

	//Returns whether or not current target has attached to target platform.
	bool isTargetAttached(void) {
		return this->targetsState[this->currentTarget].isAttached;
	}

	//Returns whether or not current target is in a valid state.
	bool isTargetInValidState(void) {
		return this->targetsState[this->currentTarget].hasValidState;
	}

	//Returns the address of the Moby Stack base.
	uint32_t getStackBase(void) {
		return this->targetsState[this->currentTarget].mobyStackBase;
	}

	//Returns the address of the maximum address the Moby Stack can reach (MOBY_STACK_MAX).
	uint32_t getStackTop(void) {
		return this->targetsState[this->currentTarget].mobyStackMax;
	}

	//Returns the current game's number
	GameID getCurrentGame(void) {
		return this->targetsState[this->currentTarget].currentGame;
	}

	//Cleans up a target's internal state - pass TARGET_PLATFORM_NONE for current target
	void cleanupTarget(int target = TARGET_PLATFORM_NONE) {
		if (target == TARGET_PLATFORM_NONE && this->currentTarget >= 0) {
			this->targetsState[this->currentTarget].target->cleanup();
			resetTargetState(&this->targetsState[this->currentTarget]);
		}
		else if (target >= 0 && target < TARGET_PLATFORMS_COUNT){
			this->targetsState[target].target->cleanup();
			resetTargetState(&this->targetsState[target]);
		}
	}

private:
	int currentTarget = TARGET_PLATFORM_NONE;
	TargetState targetsState[TARGET_PLATFORMS_COUNT] = {
		{&PS2_emu_target}, {&dummy /*PS3Emu*/}, {&dummy /*PS3*/}, {&dummy/*Vita*/}
	};

	MobyEntry mobysData1[MAX_MOBYS_COUNT] = { 0 };
	MobyEntry mobysData2[MAX_MOBYS_COUNT] = { 0 };
	MobyEntry* activeMobysBank = mobysData1;
	size_t mobysCount = 0;
};