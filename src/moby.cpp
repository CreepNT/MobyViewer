#include <stdint.h>
#include "widgets.h"

#include "moby.h"

#ifdef BIG_ENDIAN_HOST
#error Big-endian host platforms code is not implemented
#endif

#include <stdio.h>

namespace {
	static const uint8_t* workPtr = nullptr;
	static size_t readBytes = 0;
	void setPointer(const uint8_t* ptr) {
		workPtr = ptr;
		readBytes = 0;
	}
	
	template <typename T>
	T getDataFromBuffer(void) {
		static_assert(sizeof(s8) == 1, "Size of s8 must be 1.");
		static_assert(sizeof(u8) == 1, "Size of u8 must be 1.");
		static_assert(sizeof(s16) == 2, "Size of s16 must be 2.");
		static_assert(sizeof(u16) == 2, "Size of u16 must be 2.");
		static_assert(sizeof(s32) == 4, "Size of s32 must be 4.");
		static_assert(sizeof(u32) == 4, "Size of u32 must be 4.");
		static_assert(sizeof(s64) == 8, "Size of s64 must be 8.");
		static_assert(sizeof(u64) == 8, "Size of u64 must be 8.");
		static_assert(sizeof(f32) == 4, "Size of f32 must be 4.");
		static_assert(sizeof(Pointer) == 4, "Size of Pointer must be 4.");

		readBytes += sizeof(T);
		if (readBytes > MOBY_SIZE) printf("Read past buffer end !\n");
		if (workPtr == nullptr) return static_cast<T>(0);
		
		T retVal = *(reinterpret_cast<const T*>(workPtr));
		workPtr += sizeof(T);
		return retVal;
	}

	inline u8 getU8(void) { return getDataFromBuffer<u8>(); }
	inline s8 getS8(void) { return getDataFromBuffer<s8>(); }
	inline u16 getU16(void) { return getDataFromBuffer<u16>(); }
	inline s16 getS16(void) { return getDataFromBuffer<s16>(); }
	inline u32 getU32(void) { return getDataFromBuffer<u32>(); }
	inline s32 getS32(void) { return getDataFromBuffer<s32>(); }
	inline u64 getU64(void) { return getDataFromBuffer<u64>(); }
	inline f32 getFloat(void) { return getDataFromBuffer<f32>(); }
	inline Pointer getPtr(void) { return getDataFromBuffer<Pointer>(); }
	inline void getVec4(Vec4& dst) {
		dst.x = getFloat();
		dst.y = getFloat();
		dst.z = getFloat();
		dst.w = getFloat();
	}
	inline void getBSphere(BSphere& dst) {
		dst.x = getFloat();
		dst.y = getFloat();
		dst.z = getFloat();
		dst.rad = getFloat();
	}
	inline void getMtx3(Matrix3x4& dst) {
		getVec4(dst.a);
		getVec4(dst.b);
		getVec4(dst.c);
	}

	static void HostEndiannessBufferToOG3Moby(const uint8_t* buf, Moby* m) {
		m->bSphere.x = *(f32*)buf;
		m->bSphere.y = *(f32*)((uintptr_t)buf + 0x4);
		m->bSphere.z = *(f32*)((uintptr_t)buf + 0x8);
		m->bSphere.rad = *(f32*)((uintptr_t)buf + 0xC);
		m->pos.x = *(f32*)((uintptr_t)buf + 0x10);
		m->pos.y = *(f32*)((uintptr_t)buf + 0x14);
		m->pos.z = *(f32*)((uintptr_t)buf + 0x18);
		m->pos.w = *(f32*)((uintptr_t)buf + 0x1C);
		m->state = buf[0x20];
		m->group = buf[0x21];
		m->mClass = buf[0x22];
		m->alpha = buf[0x23];
		m->pClass = *(Pointer*)((uintptr_t)buf + 0x24);
		m->pChain = *(Pointer*)((uintptr_t)buf + 0x28);
		m->scale = *(f32*)((uintptr_t)buf + 0x2C);
		m->unk_30 = buf[0x30];
		m->drawn = buf[0x31];
		m->drawDist = *(uint16_t*)((uintptr_t)buf + 0x32);
		m->flags1 = *(uint16_t*)((uintptr_t)buf + 0x34);
		m->flags2 = *(uint16_t*)((uintptr_t)buf + 0x36);
		m->color1 = *(uint32_t*)((uintptr_t)buf + 0x38);
		m->color2 = *(uint32_t*)((uintptr_t)buf + 0x3C);
		m->unk_40 = *(uint32_t*)((uintptr_t)buf + 0x40);
		m->unk_44 = *(uint32_t*)((uintptr_t)buf + 0x44);
		m->unk_48 = *(f32*)((uintptr_t)buf + 0x48);
		m->unk_4C = *(f32*)((uintptr_t)buf + 0x4C);
		m->unk_50 = *(uint32_t*)((uintptr_t)buf + 0x50);
		m->unk_54 = *(uint32_t*)((uintptr_t)buf + 0x54);
		m->previousAnimation = *(uint32_t*)((uintptr_t)buf + 0x58);
		m->currentAnimation = *(uint32_t*)((uintptr_t)buf + 0x5C);
		m->unk_60 = *(uint32_t*)((uintptr_t)buf + 0x60);
		m->updateFunction = *(Pointer*)((uintptr_t)buf + 0x64);
		m->pVars = *(Pointer*)((uintptr_t)buf + 0x68);
		m->unk_6C = *(uint32_t*)((uintptr_t)buf + 0x6C);
		m->unk_70 = *(uint32_t*)((uintptr_t)buf + 0x70);
		m->unk_74 = *(uint32_t*)((uintptr_t)buf + 0x74);
		m->unk_78 = *(uint32_t*)((uintptr_t)buf + 0x78);
		m->unk_7C[0] = buf[0x7C];
		m->unk_7C[1] = buf[0x7D];
		m->unk_7C[2] = buf[0x7E];
		m->unk_7F = *(uint32_t*)((uintptr_t)buf + 0x7F);
		m->unk_83 = *(uint32_t*)((uintptr_t)buf + 0x83);
		m->unk_87 = *(uint32_t*)((uintptr_t)buf + 0x87);
		m->unk_8B = *(uint32_t*)((uintptr_t)buf + 0x8B);
		m->unk_8F = *(uint32_t*)((uintptr_t)buf + 0x8F);
		m->unk_93 = *(uint32_t*)((uintptr_t)buf + 0x93);
		m->unk_97 = buf[0x97];
		m->collData = *(Pointer*)((uintptr_t)buf + 0x98);
		m->unk_9C = *(uint32_t*)((uintptr_t)buf + 0x9C);
		m->collisionCounter = *(uint32_t*)((uintptr_t)buf + 0xA0);
		m->unk_A4 = *(uint32_t*)((uintptr_t)buf + 0xA4);
		m->unk_A8 = *(uint16_t*)((uintptr_t)buf + 0xA8);
		m->oClass = *(uint16_t*)((uintptr_t)buf + 0xAA);
		m->unk_AC = *(uint32_t*)((uintptr_t)buf + 0xAC);
		m->unk_B0 = *(uint16_t*)((uintptr_t)buf + 0xB0);
		m->UID = *(uint16_t*)((uintptr_t)buf + 0xB2);
		m->unk_B4 = *(uint32_t*)((uintptr_t)buf + 0xB4);
		m->multiMobyPart = *(Pointer*)((uintptr_t)buf + 0xB8);
		m->unk_BC = *(uint32_t*)((uintptr_t)buf + 0xBC);
		m->scaleX.x = *(f32*)((uintptr_t)buf + 0xC0);
		m->scaleX.y = *(f32*)((uintptr_t)buf + 0xC4);
		m->scaleX.z = *(f32*)((uintptr_t)buf + 0xC8);
		m->scaleX.w = *(f32*)((uintptr_t)buf + 0xCC);
		m->scaleY.x = *(f32*)((uintptr_t)buf + 0xD0);
		m->scaleY.y = *(f32*)((uintptr_t)buf + 0xD4);
		m->scaleY.z = *(f32*)((uintptr_t)buf + 0xD8);
		m->scaleY.w = *(f32*)((uintptr_t)buf + 0xDC);
		m->scaleZ.x = *(f32*)((uintptr_t)buf + 0xE0);
		m->scaleZ.y = *(f32*)((uintptr_t)buf + 0xE4);
		m->scaleZ.z = *(f32*)((uintptr_t)buf + 0xE8);
		m->scaleZ.w = *(f32*)((uintptr_t)buf + 0xEC);
		m->rotation.x = *(f32*)((uintptr_t)buf + 0xF0);
		m->rotation.y = *(f32*)((uintptr_t)buf + 0xF4);
		m->rotation.z = *(f32*)((uintptr_t)buf + 0xF8);
		m->rotation.w = *(f32*)((uintptr_t)buf + 0xFC);
	}
	//static void ReverseEndiannessBufferToOG3Moby(const uint8_t* buf, Moby* m) {}

	static void HostEndiannessBufferToRC4Moby(const uint8_t* buf, RC4Moby* m) {
		setPointer(buf);

		getBSphere(m->bSphere);
		getVec4(m->pos);
		m->state		= getS8();
		m->group		= getU8();
		m->mClass		= getS8();
		m->alpha		= getS8();
		m->pClass		= getPtr();
		m->pChain		= getPtr();
		m->collDamage	= getU8();
		m->deathCnt		= getS8();
		m->occlIndex	= getU16();
		m->updateDist	= getS8();
		m->drawn		= getS8();
		m->drawDist		= getS16();
		m->modeBits		= getU16();
		m->modeBits2	= getU16();
		m->lights		= getU64();
		m->animSeq		= getPtr();
		m->animSeqT		= getFloat();
		m->animSpeed	= getFloat();
		m->animIScale	= getS16();
		m->poseCacheEntryIndex = getS16();
		m->animLayers	= getPtr();
		m->animSeqId	= getS8();
		m->animFlags	= getS8();
		m->lSeq			= getS8();
		m->jointCnt		= getS8();
		m->jointCache	= getPtr();
		m->pManipulator = getPtr();
		m->glow_rgba	= getS32();
		m->lod_trans	= getS8();
		m->lod_trans2	= getS8();
		m->metal		= getS8();
		m->subState		= getS8();
		m->prevState	= getS8();
		m->stateType	= getS8();
		m->stateTimer	= getU16();
		m->soundTrigger = getS8();
		m->soundDesired = getS8();
		m->soundChannel = getS16();
		m->scale		= getFloat();
		m->bangles		= getU16();
		m->shadow		= getS8();
		m->shadow_index = getS8();
		m->shadow_plane = getFloat();
		m->shadow_range = getFloat();
		getBSphere(m->lSphere);
		m->netObject	= getPtr();
		m->updateID		= getS16();
		m->spad0		= getS16();
		m->collData		= getPtr();
		m->collActive	= getS32();
		m->collCnt		= getU32();
		m->grid_min_x	= getS8();
		m->grid_min_y	= getS8();
		m->grid_max_x	= getS8();
		m->grid_max_y	= getS8();
		m->pUpdate		= getPtr();
		m->pVar			= getPtr();
		m->mission		= getS8();
		m->pad			= getS8();
		m->UID			= getS16();
		m->bolts		= getS16();
		m->xp			= getU16();
		m->pParent		= getPtr();
		m->oClass		= getS16(); 
		m->triggers		= getS8();
		m->standarddeathcalled = getS8();
		getMtx3(m->rMtx);
		getVec4(m->rot);
	}
	//static void ReverseEndiannessBufferToRC4Moby(const uint8_t* buf, RC4Moby* m) {}
}

namespace MobyTool {
	void LEBufferToOG3Moby(const uint8_t* buf, Moby* m) {
#ifdef BIG_ENDIAN_HOST
		return ReverseEndiannessBufferToOG3Moby(buf, m);
#else
		return HostEndiannessBufferToOG3Moby(buf, m);
#endif
	}

	void LEBufferToRC4Moby(const uint8_t* buf, RC4Moby* m) {
#ifdef BIG_ENDIAN_HOST
		return ReverseEndiannessBufferToRC4Moby(buf, m);
#else
		return HostEndiannessBufferToRC4Moby(buf, m);
#endif
	}

	//void BEBufferToOG3Moby(const uint8_t * buf, Moby * m);
	//void BEBufferToRC4Moby(const uint8_t * buf, RC4Moby * m);
};