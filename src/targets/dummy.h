#include "target.h"
#include <stdio.h>

class DummyTarget : public Target {
	size_t readTargetMemory(uint32_t addr, uint8_t* buf, size_t readSize) {
		fprintf(stderr, "[DummyTarget] readTargetMemory(0x%08X, %p, %d)\n", addr, buf, readSize);
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