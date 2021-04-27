#ifndef SRC_TARGETS_TARGETS_H
#define SRC_TARGETS_TARGETS_H

#include <stdint.h>

enum TargetPlatforms {
	TARGET_PLATFORM_PS2EMU,
	TARGET_PLATFORM_PS3EMU,
	TARGET_PLATFORM_PS3,
	TARGET_PLATFORM_PSVITA,
	TARGET_PLATFORMS_COUNT,
	TARGET_PLATFORM_NONE = -1
};

/*
 * Abstract class that defines the API any Target should offer.
 * Targets are wrappers around a way to read memory from a given platform. They abstract all details,
 * leaving you with a single function that allows to read memory.
 *
 * Before usage, a Target should be init(). After it is no longer needed, a Target should be cleanup()'ed.
*/
class Target {
public:
	//initParams is an (optional) pointer to a custom structure of your choice.
	//You can use it to pass arguments to the Target, i.e. which port use for IPC.
	//Returns INIT_OK if the process is a success, INIT_FAIL on error.
	virtual int init(const void* initParams = nullptr) = 0;

	//addr should be platform independant (i.e. offset from address 0) and translated to target platform address internally.
	//Passing an address from target memory address space should be supported, however (i.e. 0x81000000 on PS Vita)
	virtual size_t readTargetMemory(uint32_t addr, uint8_t* buf, size_t readSize) = 0;

	//addr should be platform independant (i.e. offset from address 0) and translated to target platform address internally.
	//Passing an address from target memory address space should be supported, however (i.e. 0x81000000 on PS Vita)
	virtual size_t writeTargetMemory(uint32_t addr, uint8_t* buf, size_t writeSize) = 0;

	//When called, this should cleanup the internal state of the target.
	virtual void cleanup(void) = 0;
};

extern const char* TargetsNames[TARGET_PLATFORMS_COUNT];

//Valid return values for init() :
constexpr int INIT_OK = 0; //Return this on success
constexpr int INIT_FAIL = -1; //Return this if any error occurs
#endif