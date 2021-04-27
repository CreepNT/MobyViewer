
#include <cstdint>

#define EE_MEMORY_SIZE (32 * 1024 * 1024)
#define DBMGR_INTERNAL_FAILURE -1
#define DBMGR_INTERNAL_OK 0

namespace DBMgrInternal {
	extern "C" int getMobyStackAddressesAndGameForPS2Emu(uint8_t * EEMemory, uint32_t * pGame, uint32_t * stackBase, uint32_t * stackMax);
}