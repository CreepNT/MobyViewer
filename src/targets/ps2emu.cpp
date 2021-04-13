#ifdef _WIN32
#include <Windows.h>
#include <Psapi.h>
#else
#error PCSX2 Target cannot be built for non-Win32 platforms yet.
#endif

#include "ps2emu.h"
#include "target.h"
#include <stdio.h>

#define MAGIC (UINT32)0x3C1A8001

//TODO : Try to make this cross-platform, even when using system-specific functions.
//The ideal would be to use IPC, but it is not widespread enough and too limited for now.
//IPC support will probably be added when memory ranges reads will be supported.

namespace {
	static BOOL CALLBACK EnumWindowsCB(HWND hwnd, LPARAM lParam) {
		DWORD ret, procId;
		HANDLE procHandle;
		char execName[512];
		GetWindowThreadProcessId(hwnd, &procId);
		procHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
		if (procHandle == NULL)
			return TRUE;

		ret = GetModuleFileNameExA(procHandle, NULL, execName, sizeof(execName) / sizeof(TCHAR));
		if (ret == 0)
			return TRUE;

		char* cmpret;
		if ((cmpret = strstr(execName, "pcsx2"), cmpret != NULL) || (cmpret = strstr(execName, "PCSX2"), cmpret != NULL)) {
			printf("[PS2Emu] Found a matching process for PCSX2 with process ID 0x%lX (%ld) !\n", procId, procId);
			*(HANDLE*)lParam = procHandle;
			return FALSE;
		}
		else if (CloseHandle(procHandle) == 0) {
			DWORD error = GetLastError();
			printf("[PS2Emu] Closing HANDLE %p to process 0x%lX failed : 0x%lX.\n", procHandle, procId, error);
			return TRUE;
		}
		else return TRUE;
	}
}



int PS2EmuTarget::init(const void* initParams) {
	uint32_t ipcPort = 0;
	if (this->initialized) this->cleanup();
	if (initParams != nullptr) {
		const PS2EmuTargetInitParams* ip = (const PS2EmuTargetInitParams*)initParams;
		this->useIPC = ip->useIPC;
		ipcPort = ip->port;
	}
	if (!this->useIPC) {
		EnumWindows(EnumWindowsCB, (LPARAM)&this->PS2EmuHandle);
		if (this->PS2EmuHandle == NULL) {
			fprintf(stderr, "[PS2Emu - Target] Initialization of PS2Emu target failed, because I couldn't get a handle to the process !\n"
				"Are you sure PCSX2 is open ?\n");
			return INIT_FAIL;
		}
		else {
			UINT32 magic;
			for (ULONGLONG i = 0x20000000; i <= 0x50000000; i += 0x10000000) {
				BOOL ret = ReadProcessMemory(this->PS2EmuHandle, (LPCVOID)i, &magic, sizeof(magic), NULL);
				if (ret && magic == MAGIC) {
					printf("[PS2Emu - Target] PCSX2 EEMemory base address is 0x%08llX.\n", i);
					this->EEMemBase = (uint32_t)i;
					this->initialized = true;
					printf("[PS2Emu - Target] Target initialized with success !\n");
					return INIT_OK;
				}
			}
			fprintf(stderr, "[PS2Emu - Target] Initialization of PS2Emu target failed, because I couldn't find the EEMemory base address!\n"
					"Are you sure PCSX2 is open, and the game is launched ?\n");
			return INIT_FAIL;
		}
	}
	else {
		//TODO : add IPC init code.
		//IPC is not supported yet because getting meson to build CMake dependencies is a pain.
		fprintf(stderr, "[PS2Emu - Target] Initialization of PS2Emu target failed, because you requested to use IPC...\n");
		return INIT_FAIL;
	}
}

void PS2EmuTarget::cleanup(void) {
	if (!this->initialized) return;
	//TODO : add IPC cleanup code
	if (this->PS2EmuHandle != NULL) {
		if (CloseHandle(this->PS2EmuHandle) == 0) {
			DWORD error = GetLastError();
			fprintf(stderr, "[PS2Emu - Target] Closing HANDLE to PCSX2 process failed : 0x%lX.\n", error);
		}
		this->PS2EmuHandle = NULL;
	}
}

size_t PS2EmuTarget::readTargetMemory(uint32_t addr, uint8_t* buf, size_t readSize) {
	if (!this->initialized) {
		fprintf(stderr, "[PS2Emu - Target] Attempted to read from uninitialized target.\n");
		return 0;
	}
	uint32_t raddr = this->translateAddressToPS2AddressSpace(addr);
	if (!this->useIPC) {
		SIZE_T rs = 0;
		BOOL ret = ReadProcessMemory(this->PS2EmuHandle, (LPCVOID)raddr, (LPVOID)buf, (SIZE_T)readSize, &rs);
		return (size_t)rs;
	}
	else {
		//TODO : add IPC reading code
		return 0;
	}
}

/*
inline uint32_t PS2EmuTarget::translateAddressToPS2AddressSpace(uint32_t addr) {
	uint32_t ret = (addr & 0x3FFFFFF); //Keep only the low bytes, because max address is 0x2000000 -- This *might* not hold true for Scratchpad
	if (ret > 0x2000000) return INVALID_ADDRESS;
	else return (ret | this->EEMemBase);
}
*/

PS2EmuTarget PS2_emu_target;