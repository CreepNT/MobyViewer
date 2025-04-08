#ifndef SRC_MOBY_H
#define SRC_MOBY_H
#include <cstdint>

#define MOBY_SIZE 0x100

typedef struct Vec4 {
		float x;
		float y;
		float z;
		float w;
} Vec4;

typedef struct BSphere {
	float x;
	float y;
	float z;
	float rad;
} BSphere;

//3-rows, 4-columns matrix
typedef struct Matrix3x4 { //Called mtx3 internally
	Vec4 a;
	Vec4 b;
	Vec4 c;
} Matrix3x4;

typedef struct Matrix4x4 { //Called mtx4 internally
	Vec4 a; 
	Vec4 b;
	Vec4 c;
	Vec4 d;
} Matrix4x4;

typedef uint32_t Pointer;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;

typedef struct RC4Moby { //Internal name is MobyInstance
	BSphere bSphere; //Bounding sphere
	Vec4 pos;
	s8 state;
	u8 group;
	s8 mClass;
	s8 alpha; //0 = transparent, 128 = opaque
	Pointer pClass; //MobyClass*, some info about the model
	Pointer pChain; //MobyInstance*
	u8 collDamage;
	s8 deathCnt;
	u16 occlIndex;
	s8 updateDist; //If Moby is further than this distance, don't update it ?
	s8 drawn; //1 if drawn, 0 otherwise
	s16 drawDist; //If Moby is further than this distance, don't draw it
	u16 modeBits;
	u16 modeBits2;
	u64 lights;
	Pointer animSeq; //MobySeq*
	f32 animSeqT;
	f32 animSpeed;
	s16 animIScale;
	s16 poseCacheEntryIndex;
	Pointer animLayers; //MobyAnimLayer*
	s8 animSeqId;
	s8 animFlags;
	s8 lSeq;
	s8 jointCnt;
	Pointer jointCache; //Pointer to Matrix4x4
	Pointer pManipulator;
	s32 glow_rgba;
	s8 lod_trans;
	s8 lod_trans2;
	s8 metal;
	s8 subState;
	s8 prevState;
	s8 stateType;
	u16 stateTimer;
	s8 soundTrigger;
	s8 soundDesired;
	s16 soundChannel;
	f32 scale;
	u16 bangles;
	s8 shadow;
	s8 shadow_index;
	f32 shadow_plane;
	f32 shadow_range;
	BSphere lSphere; //Lissajous something ?
	Pointer netObject; //void*
	s16 updateID;
	s16 spad0;
	Pointer collData; //int*
	s32 collActive; 
	u32 collCnt;
	s8 grid_min_x;
	s8 grid_min_y;
	s8 grid_max_x;
	s8 grid_max_y;
	Pointer pUpdate; //Update function (void*)
	Pointer pVar; //void*
	s8 mission;
	s8 pad;
	s16 UID;
	s16 bolts;
	u16 xp;
	Pointer pParent; //MobyInstance*
	s16 oClass;
	s8 triggers;
	s8 standarddeathcalled;
	Matrix3x4 rMtx; //Matrix used for some rotation operations
	Vec4 rot; //Rotation
} RC4Moby;

typedef struct Moby {//Please note, size of unk fields may NOT be the size of actual variables (i.e. a uint32_t might be 2 uint16_t's)
	BSphere bSphere;
	Vec4 pos; //0x10
	uint8_t state; //0x20
	uint8_t group; //0x21
	uint8_t mClass; //0x22
	uint8_t alpha;	//0x23 - between 0 and 128
	Pointer pClass; //0x24 - MobyClass*
	Pointer pChain; //0x28 - Linked list of mobys that can be updated(?)
	float scale; //0x2C - scale on all axes
	uint8_t unk_30; //0x30
	uint8_t drawn; //0x31
	uint16_t drawDist; //0x32
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
	Pointer updateFunction; //0x64
	Pointer pVars; //0x68
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
	Pointer collData; //0x98
	uint32_t unk_9C; //0x9C
	uint32_t collisionCounter; //0xA0
	uint32_t unk_A4; //0xA4
	uint16_t unk_A8; //0xA8
	uint16_t oClass; //0xAA
	uint32_t unk_AC; //0xAC
	uint16_t unk_B0; //0xB0
	uint16_t UID; //0xB2
	uint32_t unk_B4; //0xB4
	Pointer multiMobyPart; //0xB8
	uint32_t unk_BC; //0xBC
	Vec4 scaleX; //0xC0
	Vec4 scaleY; //0xD0
	Vec4 scaleZ; //0xE0
	Vec4 rotation; //0xF0 - in radians
} Moby;

namespace MobyTool {
	//Convert a little-endian buffer to a Moby
	void LEBufferToOG3Moby(const uint8_t* buf, Moby* m);
	void LEBufferToRC4Moby(const uint8_t* buf, RC4Moby* m);

	//Convert a big-endian buffer to a Moby
	//void BEBufferToOG3Moby(const uint8_t * buf, Moby * m);
	//void BEBufferToRC4Moby(const uint8_t * buf, RC4Moby * m);
};
#endif