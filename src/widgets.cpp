#include <cinttypes>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include "moby.h"
#include "imgui.h"

#include "widgets.h"

typedef struct {
    uint16_t oClass;
    char* name;
} oClassString;

typedef struct {
    uint32_t stringsNum = { 0 };
    oClassString* strings = { nullptr };
} StringContainer;

static StringContainer gameStrings[GAMES_COUNT] = {};
static GameID currentGame = GAME_INVALID;
static char oClassNameBuf[512] = { 0 };

#ifndef NDEBUG
#define RESSOURCES_PATH "../data/"
#else
#define RESSOURCES_PATH
#endif

size_t GetLinesCountFromFH(FILE* fh) {
    int ret;
    size_t linesCount = 1; //Offset by one, because last line might have no \n
    char c;
    fseek(fh, 0, SEEK_SET);
    while (ret = fread(&c, 1, 1, fh), ret != EOF && ret != 0) {
        if (c == '\n') linesCount++;
    }
    fseek(fh, 0, SEEK_SET);
    return linesCount;
}

//This also strips the \n at end of line
void GetLineFromFH(FILE* fh, char* outStr, size_t maxSiz) {
    char ch, buf[2048] = { u8"Paradox ERR" };
    int ret;
    size_t bufctr = 0;
    if (maxSiz < sizeof(buf)) {
        while (bufctr < sizeof(buf) && bufctr < maxSiz) {
            ret = fread(&ch, 1, 1, fh);
            if (ret == 0 || ch == '\n') {
                if (bufctr != 0) buf[bufctr] = '\0'; //If nothing was read, we need to copy out Paradox ERR; so don't NUL the first char.
                break;
            }
            else buf[bufctr++] = ch;
        }
        buf[sizeof(buf) - 1] = '\0';
    }
    else maxSiz = sizeof(buf) - 1;
    strncpy(outStr, buf, maxSiz);
}

//Returns NULL on error, a valid address otherwise.
//NOTE : you need to free() the returned address after usage.
//allocsize is an OPTIONAL parameter, the allocation size will be copied there if specified.
char* CopyStringToMallocedSpace(char* string, size_t* allocsize = nullptr) {
    size_t alloclen = strlen(string) + 1;
    char* ret = (char*)malloc(alloclen);
    if (ret != NULL) {
        strncpy(ret, string, alloclen);
        if (allocsize != nullptr)
            *allocsize = alloclen;
        return ret;
    }
    else {
        if (allocsize != nullptr)
            *allocsize = 0;
        return nullptr;
    }
}

void loadStrings(void) {
    FILE* fh = NULL;
    for (size_t i = 0; i < GAMES_COUNT; i++) {
        char fileName[64];
        snprintf(fileName, sizeof(fileName), RESSOURCES_PATH "rc%d.txt", (i + 1));
        fh = fopen(fileName, "r");
        if (fh == NULL) {
            fprintf(stderr, "Failed to open '%s' for reading.\n", fileName);
            continue;
        }
        size_t linesCount = GetLinesCountFromFH(fh);
        oClassString* oCSptr = (oClassString*)malloc(linesCount * sizeof(oClassString));
        if (oCSptr == NULL) {
            fprintf(stderr, "Failed to allocate memory for strings from R&C%d.\n", i + 1);
            continue;
        }
        gameStrings[i].strings = oCSptr;
        gameStrings[i].stringsNum = 0;

        size_t validStringsCount = 0;
        for (size_t j = 0; j < linesCount; j++) {
            char lineBuf[512];
            GetLineFromFH(fh, lineBuf, sizeof(lineBuf));

            unsigned long oClass = 0;
            char* strPastOClass = NULL;
            oClass = strtoul(lineBuf, &strPastOClass, 16);
            if (strPastOClass == NULL || *strPastOClass != '=' || oClass > (unsigned long)UINT16_MAX) {
                fprintf(stderr, "Malformed string '%s' in file '%s' ! (strtol returned %ld)\n", lineBuf, fileName, oClass);
                continue;
            }
            else {
                oCSptr[validStringsCount].oClass = (uint16_t)oClass;
                oCSptr[validStringsCount].name = CopyStringToMallocedSpace(&strPastOClass[1] /* skip = */);
                printf("oClass %hd is '%s' for R&C%d\n", oCSptr[validStringsCount].oClass, oCSptr[validStringsCount].name, (i + 1));
                validStringsCount++;
            }
        }
        gameStrings[i].stringsNum = validStringsCount;
    }
}

void unloadStrings(void) {
    for (size_t i = 0; i < GAMES_COUNT; i++) {
        if (gameStrings[i].strings == nullptr)
            continue;
        for (size_t j = 0; j < gameStrings[i].stringsNum; j++) {
            free(gameStrings[i].strings[j].name);
        }
        free(gameStrings[i].strings);
    }
}

char const* GetGameNameFromID(GameID id) {
    switch (id) {
    case GAME_RC1:
        return "Ratchet & Clank";
    case GAME_RC2:
        return "Ratchet & Clank 2";
    case GAME_RC3:
        return "Ratchet & Clank 3";
    case GAME_RC4:
        return "Ratchet 4";
    case GAME_INVALID:
    default:
        return "Unknown/Invalid";
    }
}

char const* GetOClassStringForID(uint16_t oClass, uint32_t game) {
    if (game > GAME_RC4 || game == GAME_INVALID)
        return nullptr;
    game -= 1; //Indexes are (gameID - 1)
    for (size_t i = 0; i < gameStrings[game].stringsNum; i++) {
        if (oClass == gameStrings[game].strings[i].oClass)
            return gameStrings[game].strings[i].name;
    }
    return nullptr;
}

#define ImGui_ScalarDisplayF(name, type, data, format) ImGui::InputScalar(name, type, data, NULL, NULL, format, ImGuiInputTextFlags_ReadOnly)
#define ImGui_ScalarDisplay(name, type, data) ImGui_ScalarDisplayF(name, type, data, NULL) //Decimal print by default (%d)

#define ImGui_U8HexDisplay(name, data)      ImGui_ScalarDisplayF(name, ImGuiDataType_U8,  data, "0x%02" PRIX8)
#define ImGui_U16HexDisplay(name, data)     ImGui_ScalarDisplayF(name, ImGuiDataType_U16, data, "0x%04" PRIX16)
#define ImGui_U32HexDisplay(name, data)     ImGui_ScalarDisplayF(name, ImGuiDataType_U32, data, "0x%08" PRIX32)
#define ImGui_U64HexDisplay(name, data)     ImGui_ScalarDisplayF(name, ImGuiDataType_U64, data, "0x%016" PRIX64)
#define ImGui_PointerDisplay(name, data)    ImGui_ScalarDisplayF(name, ImGuiDataType_U32, data, "@ 0x%08" PRIX32)

void Add_rMtxWidgets(Matrix3x4* mtx) {
    ImGui::InputFloat4("rMtx.a", (float*)&mtx->a, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat4("rMtx.b", (float*)&mtx->b, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat4("rMtx.c", (float*)&mtx->c, "%.3f", ImGuiInputTextFlags_ReadOnly);
}

void AddRC4MobyWidgets(RC4Moby* m) {
    //Display important infos first (Drawn?, position, oClass, UID, state, modeBits) 
    ImGui::Checkbox("Drawn ?", (bool*)&m->drawn);
    ImGui::InputFloat3("Position", (float*)&m->pos, "%.3f", ImGuiInputTextFlags_ReadOnly);

    char const* oClassStr = GetOClassStringForID(m->oClass, GAME_RC4);
    if (oClassStr != nullptr) {
        snprintf(oClassNameBuf, sizeof(oClassNameBuf), "%s (%hd)", oClassStr, m->oClass);
        ImGui::InputText("oClass", oClassNameBuf, sizeof(oClassNameBuf), ImGuiInputTextFlags_ReadOnly);
    }
    else {
        snprintf(oClassNameBuf, sizeof(oClassNameBuf), "%hd (%04hX)", m->oClass, m->oClass);
        ImGui::InputText("oClass", oClassNameBuf, sizeof(oClassNameBuf), ImGuiInputTextFlags_ReadOnly);
    }
    ImGui_ScalarDisplay("UID", ImGuiDataType_S16, &m->UID);
    ImGui_ScalarDisplayF("State", ImGuiDataType_U8, &m->state, "%02hhX");
    ImGui_U16HexDisplay("modeBits", &m->modeBits);

    ImGui::InputFloat4("bSphere", (float*)&m->bSphere, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui_U8HexDisplay("Group", &m->group);
    ImGui_U8HexDisplay("mClass", &m->mClass);
    ImGui_U8HexDisplay("Alpha", &m->alpha);
    ImGui_PointerDisplay("pClass", &m->pClass);
    ImGui_PointerDisplay("pChain", &m->pChain);
    ImGui_U8HexDisplay("collDamage", &m->collDamage);
    ImGui_U8HexDisplay("deathCnt", &m->deathCnt);
    ImGui_U16HexDisplay("Occlusion Index", &m->occlIndex);
    ImGui_ScalarDisplay("Update Distance", ImGuiDataType_S16, &m->updateDist);
    ImGui_ScalarDisplay("Draw Distance", ImGuiDataType_S16, &m->drawDist);
    ImGui_U16HexDisplay("modeBits2", &m->modeBits2);
    ImGui_U64HexDisplay("Lights", &m->lights);

    float clr[4];
    clr[0] = ((float)(m->lights & 0xFF) / 255.f);
    clr[1] = ((float)((m->lights >> 8) & 0xFF) / 255.f);
    clr[2] = ((float)((m->lights >> 16) & 0xFF) / 255.f);
    clr[3] = ((float)((m->lights >> 24) & 0xFF) / 255.f);
    ImGui::ColorEdit4("Light 1 (LSB)", clr, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Uint8);
    clr[0] = ((float)(m->lights & 0xFF) / 255.f);
    clr[1] = ((float)((m->lights >> 32) & 0xFF) / 255.f);
    clr[2] = ((float)((m->lights >> 48) & 0xFF) / 255.f);
    clr[3] = ((float)((m->lights >> 56) & 0xFF) / 255.f);
    ImGui::ColorEdit4("Light 2 (MSB)", clr, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Uint8);

    ImGui_U32HexDisplay("animSeq", &m->animSeq);
    ImGui::InputFloat("animSeqT", &m->animSeqT, 0.f, 0.f, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat("animSpeed", &m->animSpeed, 0.f, 0.f, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui_ScalarDisplay("animIScale", ImGuiDataType_S16, &m->animIScale);
    ImGui_U16HexDisplay("poseCacheEntryIndex", &m->poseCacheEntryIndex);
    ImGui_PointerDisplay("animLayers", &m->animLayers);
    ImGui_U8HexDisplay("animSeqId", &m->animSeqId);
    ImGui_U8HexDisplay("animFlags", &m->animFlags);
    ImGui_U8HexDisplay("lSeq", &m->lSeq);
    ImGui_ScalarDisplay("jointCnt", ImGuiDataType_S8, &m->jointCnt);
    ImGui_PointerDisplay("jointCache", &m->jointCache);
    ImGui_PointerDisplay("pManipulator", &m->pManipulator);
    ImGui_U32HexDisplay("glow_rgba", &m->glow_rgba);
    ImGui_U8HexDisplay("lod_trans", &m->lod_trans);
    ImGui_U8HexDisplay("lod_trans2", &m->lod_trans2);
    ImGui_U8HexDisplay("metal", &m->metal);
    ImGui_ScalarDisplay("subState", ImGuiDataType_U8, &m->subState);
    ImGui_ScalarDisplay("prevState", ImGuiDataType_U8, &m->prevState);
    ImGui_U8HexDisplay("stateType", &m->stateType);
    ImGui_U16HexDisplay("stateTimer", &m->stateTimer);
    ImGui_U8HexDisplay("soundTrigger", &m->soundTrigger);
    ImGui_U8HexDisplay("soundDesired", &m->soundDesired);
    ImGui_U16HexDisplay("soundChannel", &m->soundChannel);
    ImGui::InputFloat("Scale", &m->scale, 0.f, 0.f, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui_U16HexDisplay("Bangles", &m->bangles);
    ImGui_U8HexDisplay("shadow", &m->shadow);
    ImGui_U8HexDisplay("shadow_index", &m->shadow_index);
    ImGui::InputFloat("shadow_plane", &m->shadow_plane, 0.f, 0.f, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat("shadow_range", &m->shadow_range, 0.f, 0.f, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat4("lSphere", (float*)&m->lSphere, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui_PointerDisplay("netObject", &m->netObject);
    ImGui_U16HexDisplay("updateID", &m->updateID);
    ImGui_U16HexDisplay("spad0",  &m->spad0);
    ImGui_PointerDisplay("collData", &m->collData);
    ImGui_U32HexDisplay("collActive", &m->collActive);
    ImGui_U32HexDisplay("collCnt", &m->collCnt);
    ImGui_ScalarDisplay("Grid Min X", ImGuiDataType_U8, &m->grid_min_x);
    ImGui_ScalarDisplay("Grid Min Y", ImGuiDataType_U8, &m->grid_min_y);
    ImGui_ScalarDisplay("Grid Max X", ImGuiDataType_U8, &m->grid_max_x);
    ImGui_ScalarDisplay("Grid Max Y", ImGuiDataType_U8, &m->grid_max_y);
    ImGui_PointerDisplay("Update Function", &m->pUpdate);
    ImGui_PointerDisplay("pVar", &m->pVar);
    ImGui_U8HexDisplay("mission", &m->mission);
    ImGui_U8HexDisplay("pad", &m->pad);
    ImGui_ScalarDisplay("bolts", ImGuiDataType_S16, &m->bolts);
    ImGui_ScalarDisplay("xp", ImGuiDataType_U16, &m->xp);
    ImGui_PointerDisplay("pParent", &m->pParent);
    ImGui_U8HexDisplay("triggers", &m->triggers);
    ImGui_U8HexDisplay("standarddeathcalled", &m->standarddeathcalled);
    Add_rMtxWidgets(&m->rMtx);
    ImGui::InputFloat4("Rotation", (float*)&m->rot, "%.3f", ImGuiInputTextFlags_ReadOnly);
}

void AddOG3MobyWidgets(Moby* m, GameID game) {
    //Display important infos first (Drawn?, position, oClass, UID, state, modeBits) 
    ImGui::Checkbox("Drawn?", (bool*)&m->drawn);
    ImGui::InputFloat4("BSphere", (float*)&m->bSphere, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Position", (float*)&m->pos, "%.3f", ImGuiInputTextFlags_ReadOnly);

    char const* oClassStr = GetOClassStringForID(m->oClass, game);
    if (oClassStr != nullptr) {
        snprintf(oClassNameBuf, sizeof(oClassNameBuf), "%s (%hd)", oClassStr, m->oClass);
        ImGui::InputText("oClass", oClassNameBuf, sizeof(oClassNameBuf), ImGuiInputTextFlags_ReadOnly);
    }
    else {
        snprintf(oClassNameBuf, sizeof(oClassNameBuf), "%hd (%04hX)", m->oClass, m->oClass);
        ImGui::InputText("oClass", oClassNameBuf, sizeof(oClassNameBuf), ImGuiInputTextFlags_ReadOnly);
    }

    ImGui::InputScalar("Moby UID", ImGuiDataType_U16, &m->UID, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("State", ImGuiDataType_U8, &m->state, NULL, NULL, "%02hhX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Group", ImGuiDataType_U8, &m->group, NULL, NULL, "%02hhX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("mClass", ImGuiDataType_U8, &m->mClass, NULL, NULL, "%02hhX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Alpha", ImGuiDataType_U8, &m->alpha, NULL, NULL, "%02hhX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("pClass", ImGuiDataType_U32, &m->pClass, NULL, NULL, "@ 0x%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("pChain", ImGuiDataType_U32, &m->pChain, NULL, NULL, "@ 0x%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat("Scale (all axes)", &m->scale, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk30", ImGuiDataType_U8, &m->unk_30, NULL, NULL, "%02hhX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Draw Distance", ImGuiDataType_S16, &m->drawDist, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Flags 1", ImGuiDataType_U16, &m->flags1, NULL, NULL, "%04hX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Flags 2", ImGuiDataType_U16, &m->flags2, NULL, NULL, "%04hX", ImGuiInputTextFlags_ReadOnly);
    float clr[4];
    clr[0] = ((float)(m->color1 & 0xFF) / 255.f);
    clr[1] = ((float)((m->color1 >> 8) & 0xFF) / 255.f);
    clr[2] = ((float)((m->color1 >> 16) & 0xFF) / 255.f);
    clr[3] = ((float)((m->color1 >> 24) & 0xFF) / 255.f);
    ImGui::ColorEdit4("Color 1", clr, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Uint8);
    clr[0] = ((float)(m->color2 & 0xFF) / 255.f);
    clr[1] = ((float)((m->color2 >> 8) & 0xFF) / 255.f);
    clr[2] = ((float)((m->color2 >> 16) & 0xFF) / 255.f);
    clr[3] = ((float)((m->color2 >> 24) & 0xFF) / 255.f);
    ImGui::ColorEdit4("Color 2", clr, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Uint8);
    ImGui::InputScalar("Unk40", ImGuiDataType_U64, (uint64_t*)&m->unk_40, NULL, NULL, "%016llX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat("Unk48", &m->unk_48, 0.f, 0.f, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat("Unk4C", &m->unk_4C, 0.f, 0.f, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk50", ImGuiDataType_U64, (uint64_t*)&m->unk_50, NULL, NULL, "%016llX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Previous animation", ImGuiDataType_U32, &m->previousAnimation, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Current animation", ImGuiDataType_U32, &m->currentAnimation, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk60", ImGuiDataType_U32, (uint32_t*)&m->unk_60, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Update function address", ImGuiDataType_U32, &m->updateFunction, NULL, NULL, "@ 0x%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("pVars address", ImGuiDataType_U32, &m->pVars, NULL, NULL, "@ 0x%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk6C", ImGuiDataType_U32, &m->unk_6C, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk70", ImGuiDataType_U32, &m->unk_70, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk74", ImGuiDataType_U32, &m->unk_74, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk78", ImGuiDataType_U32, &m->unk_78, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    uint32_t tmp = (*(uint32_t*)&m->unk_7C) & 0x00FFFFFF;
    ImGui::InputScalar("Unk7C", ImGuiDataType_U32, &tmp, NULL, NULL, "%06X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk7F", ImGuiDataType_U32, &m->unk_7F, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk83", ImGuiDataType_U32, &m->unk_83, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk87", ImGuiDataType_U32, &m->unk_87, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk8B", ImGuiDataType_U32, &m->unk_8B, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk8F", ImGuiDataType_U32, &m->unk_8F, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk93", ImGuiDataType_U32, &m->unk_93, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk97", ImGuiDataType_U32, &m->unk_97, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("collData address", ImGuiDataType_U32, &m->collData, NULL, NULL, "@ 0x%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("UNK_9C", ImGuiDataType_U32, &m->unk_9C, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Collision counter", ImGuiDataType_U32, &m->collisionCounter, NULL, NULL, "%X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("UnkA4", ImGuiDataType_U32, &m->unk_A4, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("UnkA8", ImGuiDataType_U16, &m->unk_A8, NULL, NULL, "%04hX", ImGuiInputTextFlags_ReadOnly);

    ImGui::InputScalar("UnkAC", ImGuiDataType_U32, &m->unk_AC, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("UnkB0", ImGuiDataType_U16, &m->unk_B0, NULL, NULL, "%04hX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("UnkB4", ImGuiDataType_U32, &m->unk_B4, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Multi-Moby Part pointer", ImGuiDataType_U32, &m->multiMobyPart, NULL, NULL, "@ 0x%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("UnkBC", ImGuiDataType_U32, &m->unk_BC, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Scale (X)", (float*)&m->scaleX, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Scale (Y)", (float*)&m->scaleY, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Scale (Z)", (float*)&m->scaleZ, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Rotation (rad)", (float*)&m->rotation, "%.3f", ImGuiInputTextFlags_ReadOnly);
}