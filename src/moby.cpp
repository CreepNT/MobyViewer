
#include "moby.h"
#include <stdint.h>

namespace MobyTool {
	//Convert a little-endian buffer to a Moby
	void LEBufferToMoby(const uint8_t* buf, Moby* m) {
		m->unk0.x = *(float*)buf;
		m->unk0.y = *(float*)((uintptr_t)buf + 0x4);
		m->unk0.z = *(float*)((uintptr_t)buf + 0x8);
		m->unk0.w = *(float*)((uintptr_t)buf + 0xC);
		m->pos.x = *(float*)((uintptr_t)buf + 0x10);
		m->pos.y = *(float*)((uintptr_t)buf + 0x14);
		m->pos.z = *(float*)((uintptr_t)buf + 0x18);
		m->pos.w = *(float*)((uintptr_t)buf + 0x1C);
		m->state = buf[0x20];
		m->group = buf[0x21];
		m->textureMode = buf[0x22];
		m->opacity = buf[0x23];
		m->model = *(Pointer32*)((uintptr_t)buf + 0x24);
		m->parentMoby = *(Pointer32*)((uintptr_t)buf + 0x28);
		m->scale = *(float*)((uintptr_t)buf + 0x2C);
		m->unk_30 = buf[0x30];
		m->visible = buf[0x31];
		m->renderDistance = *(uint16_t*)((uintptr_t)buf + 0x32);
		m->flags1 = *(uint16_t*)((uintptr_t)buf + 0x34);
		m->flags2 = *(uint16_t*)((uintptr_t)buf + 0x36);
		m->color1 = *(uint32_t*)((uintptr_t)buf + 0x38);
		m->color2 = *(uint32_t*)((uintptr_t)buf + 0x3C);
		m->unk_40 = *(uint32_t*)((uintptr_t)buf + 0x40);
		m->unk_44 = *(uint32_t*)((uintptr_t)buf + 0x44);
		m->unk_48 = *(float*)((uintptr_t)buf + 0x48);
		m->unk_4C = *(float*)((uintptr_t)buf + 0x4C);
		m->unk_50 = *(uint32_t*)((uintptr_t)buf + 0x50);
		m->unk_54 = *(uint32_t*)((uintptr_t)buf + 0x54);
		m->previousAnimation = *(uint32_t*)((uintptr_t)buf + 0x58);
		m->currentAnimation = *(uint32_t*)((uintptr_t)buf + 0x5C);
		m->unk_60 = *(uint32_t*)((uintptr_t)buf + 0x60);
		m->updateFunction = *(Pointer32*)((uintptr_t)buf + 0x64);
		m->pVars = *(Pointer32*)((uintptr_t)buf + 0x68);
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
		m->collData = *(Pointer32*)((uintptr_t)buf + 0x98);
		m->unk_9C = *(uint32_t*)((uintptr_t)buf + 0x9C);
		m->collisionCounter = *(uint32_t*)((uintptr_t)buf + 0xA0);
		m->unk_A4 = *(uint32_t*)((uintptr_t)buf + 0xA4);
		m->unk_A8 = *(uint16_t*)((uintptr_t)buf + 0xA8);
		m->oClass = *(uint16_t*)((uintptr_t)buf + 0xAA);
		m->unk_AC = *(uint32_t*)((uintptr_t)buf + 0xAC);
		m->unk_B0 = *(uint16_t*)((uintptr_t)buf + 0xB0);
		m->UID = *(uint16_t*)((uintptr_t)buf + 0xB2);
		m->unk_B4 = *(uint32_t*)((uintptr_t)buf + 0xB4);
		m->multiMobyPart = *(Pointer32*)((uintptr_t)buf + 0xB8);
		m->unk_BC = *(uint32_t*)((uintptr_t)buf + 0xBC);
		m->scaleX.x = *(float*)((uintptr_t)buf + 0xC0);
		m->scaleX.y = *(float*)((uintptr_t)buf + 0xC4);
		m->scaleX.z = *(float*)((uintptr_t)buf + 0xC8);
		m->scaleX.w = *(float*)((uintptr_t)buf + 0xCC);
		m->scaleY.x = *(float*)((uintptr_t)buf + 0xD0);
		m->scaleY.y = *(float*)((uintptr_t)buf + 0xD4);
		m->scaleY.z = *(float*)((uintptr_t)buf + 0xD8);
		m->scaleY.w = *(float*)((uintptr_t)buf + 0xDC);
		m->scaleZ.x = *(float*)((uintptr_t)buf + 0xE0);
		m->scaleZ.y = *(float*)((uintptr_t)buf + 0xE4);
		m->scaleZ.z = *(float*)((uintptr_t)buf + 0xE8);
		m->scaleZ.w = *(float*)((uintptr_t)buf + 0xEC);
		m->rotation.x = *(float*)((uintptr_t)buf + 0xF0);
		m->rotation.y = *(float*)((uintptr_t)buf + 0xF4);
		m->rotation.z = *(float*)((uintptr_t)buf + 0xF8);
		m->rotation.w = *(float*)((uintptr_t)buf + 0xFC);
	}

	//Convert a big-endian buffer to a Moby
	//void BEBufferToMoby(const uint8_t* buf, Moby* m);

	/*
	static void MobyToLEBuffer(const Moby* m, uint8_t* buf);
	static void MobyToBEBuffer(const Moby* m, uint8_t* buf);
	*/
};