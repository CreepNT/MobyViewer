#include <cstdint>
#include <cstddef>
#include <cstdio>

#ifdef _WIN32
#include <Windows.h>
#include <Psapi.h>
#endif

#include "pcsx2_ipc.h"
#include "ps2emu.h"
#include "target.h"

#define MAGIC (UINT32)0x3C1A8001

//TODO : Try to make this cross-platform, even when using system-specific functions.
//The ideal would be to use IPC, but it is not widespread enough and too limited for now.
//IPC support will probably be added when memory ranges reads will be supported.

#ifdef _WIN32
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
#endif


int PS2EmuTarget::init(const void* initParams) {
	uint32_t ipcPort = 0;
	if (this->initialized) this->cleanup();
	if (initParams != nullptr) {
		const PS2EmuTargetInitParams* ip = (const PS2EmuTargetInitParams*)initParams;
		this->useIPC = ip->useIPC;
		ipcPort = ip->port;
	}
	if (!this->useIPC) {
#ifdef _WIN32
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
			/* YUCK! But resolving "EEMem" seems complicated... */
			for (ULONGLONG i = 0x00007FF000000000ULL; i < 0x00007FFF00000000ULL; i += 0x1000000ULL) {
				BOOL ret = ReadProcessMemory(this->PS2EmuHandle, (LPCVOID)i, &magic, sizeof(magic), NULL);
				if (ret && magic == MAGIC) {
					printf("[PS2Emu - Target] PCSX2 EEMemory base address is 0x%08llX (extended).\n", i);
					this->EEMemBase = (uintptr_t)i; //cast silences warning on 32-bit, but this is dead code anyways...
					this->initialized = true;
					printf("[PS2Emu - Target] Target initialized with success !\n");
					return INIT_OK;
				}
			}
			fprintf(stderr, "[PS2Emu - Target] Initialization of PS2Emu target failed, because I couldn't find the EEMemory base address!\n"
					"Are you sure PCSX2 is open, and the game is launched ?\n");
			return INIT_FAIL;
		}
#else
		fprintf(stderr, "[PS2Emu - Target] Initialization failed, because no-IPC mode is only supported on Windows platforms.\n");
		return INIT_FAIL;
#endif
	}
	else {
		this->IpcHandle = new PCSX2Ipc((ipcPort == 0) ? 28011 : ipcPort);
		if (this->IpcHandle == nullptr) {
			fprintf(stderr, "[PS2Emu - Target] Failed to create PCSX2Ipc object.\n");
			return INIT_FAIL;
		}
		else {
			try {
				printf("[PS2Emu - Target] Version from IPC : %s\n", this->IpcHandle->Version());
			}
			catch (...) {
				fprintf(stderr, "[PS2Emu - Target] Caught expection when getting version from IPC - assuming init failed.\n");
				delete this->IpcHandle;
				this->IpcHandle = nullptr;
				return INIT_FAIL;
			}
			printf("[PS2Emu - Target] Target initialized with success !\n");
			this->initialized = true;
			return INIT_OK;
		}
	}
}

void PS2EmuTarget::cleanup(void) {
	if (!this->initialized) return;

#ifdef _WIN32
	if (this->PS2EmuHandle != NULL) {
		if (CloseHandle(this->PS2EmuHandle) == 0) {
			DWORD error = GetLastError();
			fprintf(stderr, "[PS2Emu - Target] Closing HANDLE to PCSX2 process failed : 0x%lX.\n", error);
		}
		this->PS2EmuHandle = NULL;
	}
#endif

	if (this->IpcHandle != nullptr) {
		delete this->IpcHandle;
		this->IpcHandle = nullptr;
	}
	this->initialized = false;
}

size_t PS2EmuTarget::readTargetMemory(uint32_t addr, uint8_t* buf, size_t readSize) {
	if (!this->initialized) {
		fprintf(stderr, "[PS2Emu - Target] Attempted to read from uninitialized target.\n");
		return 0;
	}
	uintptr_t raddr = this->translateAddressToPS2AddressSpace(addr);
	if (!this->useIPC) {
#ifdef _WIN32
		SIZE_T rs = 0;
		BOOL ret = ReadProcessMemory(this->PS2EmuHandle, (LPCVOID)raddr, (LPVOID)buf, (SIZE_T)readSize, &rs);
		return (size_t)rs;
#endif
	}
	else {
		this->IpcHandle->InitializeBatch();
		for (size_t i = 0; i < readSize; i++) {
			this->IpcHandle->Read<uint8_t, true>((uint32_t)(raddr + i));
		}
		auto res = this->IpcHandle->FinalizeBatch();
		this->IpcHandle->SendCommand(res);
		for (int i = 0; i < (int)readSize; i++) {
			buf[i] = this->IpcHandle->GetReply<PCSX2Ipc::MsgRead8>(res, i);
		}
		return readSize;
	}
}

size_t PS2EmuTarget::writeTargetMemory(uint32_t addr, uint8_t* buf, size_t writeSize) {
	if (!this->initialized) {
		fprintf(stderr, "[PS2Emu - Target] Attempted to write to uninitialized target.\n");
		return 0;
	}
	uintptr_t raddr = this->translateAddressToPS2AddressSpace(addr);
	if (!this->useIPC) {
#ifdef _WIN32
		SIZE_T rs = 0;
		BOOL ret = WriteProcessMemory(this->PS2EmuHandle, (LPVOID)raddr, (LPCVOID)buf, (SIZE_T)writeSize, &rs);
		return (size_t)rs;
#endif
	}
	else {
		this->IpcHandle->InitializeBatch();
		for (size_t i = 0; i < writeSize; i++) {
			this->IpcHandle->Write<uint8_t, true>((uint32_t)(raddr + i), buf[i]);
		}
		auto res = this->IpcHandle->FinalizeBatch();
		this->IpcHandle->SendCommand(res);
		return writeSize;
	}
}

PS2EmuTarget PS2_emu_target;