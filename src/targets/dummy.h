#include "target.h"
#include <cstdio>

class DummyTarget : public Target {
	size_t readTargetMemory(uint32_t addr, uint8_t* buf, size_t readSize) {
		fprintf(stderr, "[DummyTarget] readTargetMemory(0x%08X, %p, %zu)\n", addr, buf, readSize);
		return 0;
	}
	size_t writeTargetMemory(uint32_t addr, uint8_t* buf, size_t writeSize) {
		fprintf(stderr, "[DummyTarget] writeTargetMemory(0x%08X, %p, %zu)\n", addr, buf, writeSize);
		return 0;
	}
	int init(const void* initParams = nullptr) {
		fprintf(stderr, "[DummyTarget] init(%p)\n", initParams);
		return INIT_OK;
	}
	void cleanup(void) { 
		fprintf(stderr, "[DummyTarget] cleanup()\n");
		return;
	}
};