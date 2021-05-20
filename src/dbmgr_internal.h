


#define EE_MEMORY_SIZE (32 * 1024 * 1024)

//Return codes
#define DBMGR_INTERNAL_FAILURE -1
#define DBMGR_INTERNAL_OK 0

#define INVALID_GAME_NUM 0 //Each game maps to its own number otherwise

#ifdef __cplusplus //Allows to import this header in C files, for the defines
#include <cstdint>
namespace DBMgrInternal {
	extern "C" int getMobyStackAddressesAndGameForPS2Emu(uint8_t * EEMemory, uint32_t * pGame, uint32_t * stackBase, uint32_t * stackMax);
}
#endif