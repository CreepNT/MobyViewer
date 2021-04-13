//Copyright 2021 chaoticgd & CreepNT
//Tweaked implementation of chaoticgd's memmap.c from Wrench.
//See : https://github.com/chaoticgd/wrench/blob/master/src/cli/memmap.c

//This file is made avaliable under 3-Clause BSD. See LICENSE.TXT

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define GAME_COUNT         4
#define MIN_SEGMENT_COUNT  10
#define RAC1_SEGMENT_COUNT 40 // Not sure.
#define RAC2_SEGMENT_COUNT 35
#define RAC3_SEGMENT_COUNT 36
#define DL_SEGMENT_COUNT   53
#define EE_MEMORY_SIZE     (32 * 1024 * 1024)
#define BATCH_READ_SIZE    1024
#define KERNEL_BASE        0x0
#define CODE_SEGMENT_BASE  0x100000

// Caution: Deadlocked contains the R&C3 pattern.
static const char* PATTERNS[GAME_COUNT] = {
	"IOPRP243.IMG", "IOPRP255.IMG", "Ratchet and Clank: Up Your Arsenal", "Ratchet: Deadlocked"
};
static const uint32_t SEGMENT_COUNTS[GAME_COUNT] = {
	RAC1_SEGMENT_COUNT, RAC2_SEGMENT_COUNT, RAC3_SEGMENT_COUNT, DL_SEGMENT_COUNT
};
static const char* RAC1_SEGMENT_LABELS[RAC1_SEGMENT_COUNT] = {
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", ""
};

static const char* RAC2_SEGMENT_LABELS[RAC2_SEGMENT_COUNT] = {
	"OS",
	"Code",
	"",
	"",
	"",
	"",
	"",
	"Tfrag Geometry",
	"Occlusion",
	"Sky",
	"Collision",
	"Shared VRAM",
	"Particle VRAM",
	"Effects VRAM",
	"Mobies",
	"Ties",
	"Shrubs",
	"Ratchet Seqs",
	"",
	"Help Messages",
	"Tie Instances",
	"Shrub Instances",
	"Moby Instances",
	"Moby Pvars",
	"Misc Instances",
	"",
	"",
	"",
	"",
	"",
	"",
	"HUD",
	"GUI",
	"",
	""
};

static const char* RAC3_SEGMENT_LABELS[RAC3_SEGMENT_COUNT] = {
	"OS",
	"Code",
	"",
	"",
	"",
	"",
	"",
	"Tfrag Geometry",
	"Occlusion",
	"Sky",
	"Collision",
	"Shared VRAM",
	"Particle VRAM",
	"Effects VRAM",
	"Mobies",
	"Ties",
	"Shrubs",
	"Ratchet Seqs",
	"",
	"Help Messages",
	"Tie Instances",
	"Shrub Instances",
	"Moby Instances",
	"Moby Pvars",
	"Misc Instances",
	"",
	"",
	"",
	"",
	"",
	"",
	"", // R&C2 doesn't have this.
	"HUD",
	"GUI",
	"",
	""
};

static const char* DL_SEGMENT_LABELS[DL_SEGMENT_COUNT] = {
	"OS",
	"Code",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"Tfrag Geometry",
	"Occlusion",
	"Sky",
	"Collision",
	"Shared VRAM",
	"Particle VRAM",
	"Effects VRAM",
	"Mobies",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"Help Messages",
	"Tie Instances",
	"",
	"Moby Instances",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"HUD",
	"",
	"",
	"",
	""
};
static const char** SEGMENT_LABELS[GAME_COUNT] = {
	RAC1_SEGMENT_LABELS, RAC2_SEGMENT_LABELS, RAC3_SEGMENT_LABELS, DL_SEGMENT_LABELS
};

int detect_game(uint8_t* ee_memory) {
	int i, j, k;
	for (i = GAME_COUNT - 1; i >= 0; i--) {
		int game_matches = 0;
		int pattern_size = strlen(PATTERNS[i]);
		for (j = CODE_SEGMENT_BASE; j < EE_MEMORY_SIZE - 0x1000; j++) {
			int address_matches = 1;
			for (k = 0; k < pattern_size; k++) {
				if (ee_memory[j + k] != PATTERNS[i][k]) {
					address_matches = 0;
					break;
				}
			}
			if (address_matches) {
				game_matches = 1;
			}
		}
		if (game_matches) {
			return i;
		}
	}
	return -1;
}

int getCurrentGameForPS2Emu(uint8_t* eeMemory) {
	int ret = detect_game(eeMemory);
	if (ret != -1) return (ret + 1); //Enum is offset by 1 from detect_game's return
	else return ret;
}

void getMobyStackAddressesAndGameForPS2Emu(uint8_t* eeMemory, uint32_t* pGame, uint32_t* stackBase, uint32_t* stackMaxTop) {
	int game = detect_game(eeMemory);
	if (game < 0 || game > 3) {
		fprintf(stderr, "[PS2Emu - memmap] Couldn't detect game. (%d)\n", game);
		*pGame = 0; //GAME_INVALID
		return;
	}
	else {
		*pGame = (game + 1); //Game is its number (RaC1 => 1, RaC2 => 2, etc...)
	}

	uint32_t i, j;
	for (i = CODE_SEGMENT_BASE / 0x4; i < EE_MEMORY_SIZE / 4 - SEGMENT_COUNTS[game]; i++) {
		uint32_t* ptr = (uint32_t*)((uintptr_t)eeMemory + i);

		// The PS2 kernel and code segments are always at the same addresses.
		if (ptr[0] != KERNEL_BASE || ptr[1] != CODE_SEGMENT_BASE) {
			continue;
		}

		// The addresses must be in ascending order.
		int should_skip = 0;
		for (j = 0; j < 5; j++) {
			if (ptr[j] > ptr[j + 1] || ptr[j] > EE_MEMORY_SIZE) {
				should_skip = 1;
			}
		}
		if (should_skip) {
			continue;
		}

		for (j = 0; j < SEGMENT_COUNTS[game]; j++) {
			/*int32_t size;
			if (ptr[j] == 0 || ptr[j + 1] < ptr[j]) {
				size = -1;
			}
			else if (j == SEGMENT_COUNTS[game] - 1) {
				size = EE_MEMORY_SIZE - ptr[j];
			}
			else {
				
			}
			printf("%08x %-16s%08x", i + (j * 4), SEGMENT_LABELS[game][j], ptr[j]);
			if (size == -1) {
				printf("     ??? KiB\n");
			}
			else {
				printf("%8d k\n", size / 1024);
			}*/
			if (strcmp("Moby Instances", SEGMENT_LABELS[game][j]) == 0) { //Yes, this is disgustingly hacky. Deal with it.
				uint32_t KiBsize = (ptr[j + 1] - ptr[j]) / 1024;
				*stackBase = (uintptr_t)i + ((uintptr_t)j * 4);
				*stackMaxTop = (uintptr_t)i + (((uintptr_t)j + 1) * 4);
				printf("[PS2Emu - memmap] Moby Stack max size is %d KiB.\n", KiBsize);
				printf("[PS2Emu - memmap] PTR to base @ %08X, PTR to top @ %08X\n", *stackBase, *stackMaxTop);
				return;
			}
		}
		return;
	}
	fprintf(stderr, "[PS2Emu - memmap] Failed to find memory map.\n");
}