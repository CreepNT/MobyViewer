
#include <stdint.h>

#define EE_MEMORY_SIZE (32 * 1024 * 1024)

namespace DBMgrInternal {
	extern "C" void getMobyStackAddressesAndGameForPS2Emu(uint8_t * EEMemory, uint32_t * pGame, uint32_t * stackBase, uint32_t * stackMaxTop);
}