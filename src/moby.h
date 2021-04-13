#ifndef SRC_MOBY_H
#define SRC_MOBY_H
#include <stdint.h>

#define MOBY_SIZE 0x100

typedef struct Vec4 {
		float x;
		float y;
		float z;
		float w;
} Vec4;

typedef uint32_t Pointer32;

typedef struct Moby {//Please note, size of unk fields may NOT be the size of actual variables (i.e. a uint32_t might be 2 uint16_t's)
	Vec4 unk0;
	Vec4 pos; //0x10
	uint8_t state; //0x20
	uint8_t group; //0x21
	uint8_t textureMode; //0x22
	uint8_t opacity;	//0x23 - between 0 and 128
	Pointer32 model; //0x24
	Pointer32 parentMoby; //0x28 - name is probably incorrect
	float scale; //0x2C - scale on all axes
	uint8_t unk_30; //0x30
	uint8_t visible; //0x31
	uint16_t renderDistance; //0x32
	uint16_t flags1; //0x34
	uint16_t flags2; //0x36

	//Maybe RGBA
	uint32_t color1; //0x38

	/*
	* This may only apply to UYA.
	*
	* Stores 4 different values encoded in the following format :
	*	(var & 0xFF) -> brightness/shine
	*	(var & 0xFF00) -> blue
	*	(var & 0xFF0000) -> green
	*	(var & 0xFF000000) -> red
	*/
	uint32_t color2; //0x3C

	uint32_t unk_40; //0x40
	uint32_t unk_44; //0x44
	float unk_48; //0x48 - set to 1.0 in MB_initFields, and generaly left untouched
	float unk_4C; //0x4C
	uint32_t unk_50; //0x50
	uint32_t unk_54; //0x54
	uint32_t previousAnimation; //0x58
	uint32_t currentAnimation;  //0x5C
	uint32_t unk_60; //0x60
	Pointer32 updateFunction; //0x64
	Pointer32 pVars; //0x68
	uint32_t unk_6C; //0x6C
	uint32_t unk_70; //0x70 
	uint32_t unk_74; //0x74
	uint32_t unk_78; //0x78
	uint8_t unk_7C[0x3]; //0x7C
	uint32_t unk_7F; //0x7F
	uint32_t unk_83; //0x83 
	uint32_t unk_87; //0x87
	uint32_t unk_8B; //0x8B
	uint32_t unk_8F; //0x8F
	uint32_t unk_93; //0x93
	uint8_t unk_97; //0x97
	Pointer32 collData; //0x98
	uint32_t unk_9C; //0x9C
	uint32_t collisionCounter; //0xA0
	uint32_t unk_A4; //0xA4
	uint16_t unk_A8; //0xA8
	uint16_t oClass; //0xAA
	uint32_t unk_AC; //0xAC
	uint16_t unk_B0; //0xB0
	uint16_t UID; //0xB2
	uint32_t unk_B4; //0xB4
	Pointer32 multiMobyPart; //0xB8
	uint32_t unk_BC; //0xBC
	Vec4 scaleX; //0xC0
	Vec4 scaleY; //0xD0
	Vec4 scaleZ; //0xE0
	Vec4 rotation; //0xF0 - in radians
} Moby;

namespace MobyTool {
	//Convert a little-endian buffer to a Moby
	void LEBufferToMoby(const uint8_t* buf, Moby* m);

	//Convert a big-endian buffer to a Moby
	//void BEBufferToMoby(const uint8_t* buf, Moby* m);

	/*
	static void MobyToLEBuffer(const Moby* m, uint8_t* buf);
	static void MobyToBEBuffer(const Moby* m, uint8_t* buf);
	*/
};
#endif