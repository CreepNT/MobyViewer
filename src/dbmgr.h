
#include <stdint.h>
#include <string.h>

#include "moby.h"
#include "targets.h"
#include "targets/ps2emu.h"

constexpr uint32_t MAX_MOBYS_COUNT = 0x1000; //Maybe more ?
extern DummyTarget dummy;

class DBMgr {
public:
	//Returns the number of Mobys in database
	unsigned int getMobysCount(void) {
		return this->mobysCount;
	}

	//Copies a moby's data to the provided area. m is not written to if entryNum is invalid.
	void getMobyContent(unsigned int entryNum, Moby* m) {
		if (entryNum < mobysCount)
			memcpy(m, &this->activeMobysBank[entryNum], sizeof(Moby));
	}

	//Returns the pointer to selected moby, or nullptr if entryNum is invalid.
	Moby* getMobyPointer(unsigned int entryNum) {
		if (entryNum < mobysCount)
			return &(this->activeMobysBank[entryNum]);
		else return nullptr;
	}

	//Refreshes data in the database. Call this to read target's memory again.
	void refresh(void);

	//Refresh data in the database, and all possible state variables used.
	//Call this when i.e. switching game/planets on PS2.
	//Warning : this may be SIGNIFICANTLY slower than just a refreshDB(), only call this if you know what you're doing.
	void refreshStateAndDB(void);

	//Changes the target platform. Specifying an invalid target will set it to TARGET_PLATFORM_NONE.
	void setNewTarget(int target) {
		if (target >= 0 && target < TARGET_PLATFORMS_COUNT)
			this->currentTarget = target;
		else this->currentTarget = TARGET_PLATFORM_NONE;
	}

	//Returns the current target
	int getCurrentTarget(void) {
		return this->currentTarget;
	}

	//Calls the target platform's initialize function.
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

	//Returns the address of the Moby Stack base.
	uint32_t getStackBase(void) {
		return (this->targetsState[this->currentTarget].isAttached) ? this->targetsState[this->currentTarget].mobyStackBase : 0;
	}

	//Returns the address of the Moby Stack top. Warning : this may be higher than (stackBase + mobysCount * MOBY_SIZE).
	uint32_t getStackTop(void) {
		return (this->targetsState[this->currentTarget].isAttached) ? this->targetsState[this->currentTarget].mobyStackTop : 0;
	}

	//Returns the current game's number (0 = Invalid, 1 = RaC1, etc)
	uint32_t getCurrentGame(void) {
		return (this->targetsState[this->currentTarget].isAttached) ? this->targetsState[this->currentTarget].currentGame : GAME_INVALID;
	}

	void cleanupTarget(int target = TARGET_PLATFORM_NONE) {
		if (target == TARGET_PLATFORM_NONE) {
			this->targetsState[this->currentTarget].target->cleanup();
			this->targetsState[this->currentTarget].isAttached = false;
			this->targetsState[this->currentTarget].currentGame = GAME_INVALID;
		}
		else if (target >= 0 && target < TARGET_PLATFORMS_COUNT){
			this->targetsState[target].target->cleanup();
			this->targetsState[target].isAttached = false;
			this->targetsState[target].currentGame = GAME_INVALID;
		}
	}

private:
	int currentTarget = TARGET_PLATFORM_NONE;
	struct TargetState {
		Target* target;
		bool isAttached = false;
		uint32_t mobyStackBase = 0, mobyStackTop = 0, currentGame = GAME_INVALID;
	} targetsState[TARGET_PLATFORMS_COUNT] = {
		{&PS2_emu_target}, {&dummy /*PS3Emu*/}, {&dummy /*PS3*/}, {&dummy/*Vita*/}
	};

	Moby mobysData1[MAX_MOBYS_COUNT] = { 0 };
	Moby mobysData2[MAX_MOBYS_COUNT] = { 0 };
	Moby* activeMobysBank = mobysData1;
	unsigned int mobysCount = 0;
};