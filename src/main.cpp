
#include "ogl_imgui.h"
#include "dbmgr.h"
#include "moby.h"
#include "targets.h"
#include "icon.h"

#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <vector>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#ifndef NDEBUG
#define RESSOURCES_PATH "../data/"
#else
#define RESSOURCES_PATH
#endif

float DISTANCE(Moby* a, Moby* b) {
    return sqrtf(powf(b->pos.x - a->pos.x, 2.0) + powf(b->pos.y - a->pos.y, 2.0) + powf(b->pos.z - a->pos.z, 2.0));
}

char* getGameNameFromID(int id) {
    switch (id) {
    case GAME_RC1:
        return "Ratchet & Clank";
    case GAME_RC2:
        return "Ratchet & Clank 2";
    case GAME_RC3:
        return "Ratchet & Clank 3";
    case GAME_RC4:
        return "Ratchet : Gladiator";
    case GAME_INVALID:
    default:
        return "Unknown/Invalid";
    }
}

typedef struct {
    uint16_t oClass;
    char* name;
} oClassString;

typedef struct {
    uint32_t stringsNum;
    oClassString* strings;
} StringContainer;

StringContainer gameStrings[GAMES_COUNT] = {
    {0, nullptr},
    {0, nullptr},
    {0, nullptr},
    {0, nullptr}
};

size_t getLinesCountFromFH(FILE* fh) {
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
void getLineFromFH(FILE* fh, char* outStr, size_t maxSiz) {
    char ch, buf[2048] = { u8"Paradox ERR" };
    int ret, bufctr = 0;
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
char* copyStringToMallocedSpace(char* string, size_t* allocsize = nullptr) {
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

char* getOClassStringForID(uint16_t oClass, uint32_t game) {
    if (game > GAME_RC4 || game == GAME_INVALID)
        return nullptr;
    game -= 1;
    for (int i = 0; i < gameStrings[game].stringsNum; i++) {
        if (oClass == gameStrings[game].strings[i].oClass)
            return gameStrings[game].strings[i].name;
    }
    return nullptr;
}

void loadStrings(){
    FILE* fh = NULL;
    for (int i = 0; i < GAMES_COUNT; i++) {
        char fileName[64];
        snprintf(fileName, sizeof(fileName), RESSOURCES_PATH "rc%d.txt", (i + 1));
        fh = fopen(fileName, "r");
        if (fh == NULL) {
            fprintf(stderr, "Failed to open '%s' for reading.\n", fileName);
            continue;
        }
        size_t linesCount = getLinesCountFromFH(fh);
        oClassString* sp = (oClassString*)malloc(linesCount * sizeof(oClassString));
        if (sp == NULL) {
            fprintf(stderr, "Failed to allocate memory for strings from R&C%d.\n", i + 1);
            continue;
        }
        gameStrings[i].strings = sp;
        gameStrings[i].stringsNum = linesCount;
        for (int j = 0; j < linesCount; j++) {
            char lineBuf[512], strBuf[512];
            uint16_t oClass;
            getLineFromFH(fh, lineBuf, sizeof(lineBuf));
            int ret = sscanf(lineBuf, "%04hX=%s", &oClass, strBuf);
            if (ret != 2) {
                fprintf(stderr, "Malformed string '%s' in file '%s' !\n", lineBuf, fileName);
                gameStrings[i].strings[j].oClass = 0xFFFF; //Hopefully this doesn't collide with anything
                gameStrings[i].strings[j].name = "Paradox ERR";
                continue;
            }
            gameStrings[i].strings[j].name = copyStringToMallocedSpace(strBuf);
            gameStrings[i].strings[j].oClass = oClass;
            printf("oClass %hd is '%s' for R&C%d\n", oClass, strBuf, (i + 1));
        }
    }
}

void unloadStrings(){
    for (int i = 0; i < GAMES_COUNT; i++) {
        if (gameStrings[i].strings == nullptr)
            continue;
        for (int j = 0; j < gameStrings[i].stringsNum; j++) {
            if (gameStrings[i].strings[j].oClass != 0xFFFF)
                free(gameStrings[i].strings[j].name);
        }
        free(gameStrings[i].strings);
    }
}

void AddMobyWidget(Moby* m, uint32_t game) {
    ImGui::Checkbox("Visible", (bool*)&m->visible);
    ImGui::InputFloat4("Unk0", (float*)&m->unk0, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Position", (float*)&m->pos, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("State", ImGuiDataType_U8, &m->state, NULL, NULL, "%02hhX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Group", ImGuiDataType_U8, &m->group, NULL, NULL, "%02hhX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Texture Mode", ImGuiDataType_U8, &m->textureMode, NULL, NULL, "%02hhX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Opacity", ImGuiDataType_U8, &m->opacity, NULL, NULL, "%02hhX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Model Pointer", ImGuiDataType_U32, &m->model, NULL, NULL, "@ 0x%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Parent Moby Pointer", ImGuiDataType_U32, &m->parentMoby, NULL, NULL, "@ 0x%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat("Scale (all axes)", &m->scale, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Unk30", ImGuiDataType_U8, &m->unk_30, NULL, NULL, "%02hhX", ImGuiInputTextFlags_ReadOnly); 
    ImGui::InputScalar("Render Distance", ImGuiDataType_S16, &m->renderDistance, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Flags 1", ImGuiDataType_U16, &m->flags1, NULL, NULL, "%04hX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Flags 2", ImGuiDataType_U16, &m->flags2, NULL, NULL, "%04hX", ImGuiInputTextFlags_ReadOnly);
    float clr[4];
    clr[0] = ((float)(m->color1 & 0xFF) / 255.f);
    clr[1] = ((float)((m->color1 >> 8 ) & 0xFF) / 255.f);
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
    ImGui::InputScalar("pVars address", ImGuiDataType_U32 , &m->pVars, NULL, NULL, "@ 0x%08X", ImGuiInputTextFlags_ReadOnly);
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
    ImGui::InputScalar("Collision counter", ImGuiDataType_U32 , &m->collisionCounter, NULL, NULL, "%X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("UnkA4", ImGuiDataType_U32, &m->unk_A4, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("UnkA8", ImGuiDataType_U16, &m->unk_A8, NULL, NULL, "%04hX", ImGuiInputTextFlags_ReadOnly);
    
    char* oClassStr = getOClassStringForID(m->oClass, game), buf[512];
    if (oClassStr != nullptr) {
        snprintf(buf, sizeof(buf), "%s (%hd)", oClassStr, m->oClass);
        ImGui::InputText("Moby oClass", buf, strlen(buf), ImGuiInputTextFlags_ReadOnly);
    }
    else
        ImGui::InputScalar("Moby oClass", ImGuiDataType_U16, &m->oClass, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
    
    
    ImGui::InputScalar("UnkAC", ImGuiDataType_U32, &m->unk_AC, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("UnkB0", ImGuiDataType_U16, &m->unk_B0, NULL, NULL, "%04hX", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Moby UID", ImGuiDataType_U16, &m->UID, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("UnkB4", ImGuiDataType_U32, &m->unk_B4, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("Multi-Moby Part pointer", ImGuiDataType_U32, &m->multiMobyPart, NULL, NULL, "@ 0x%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputScalar("UnkBC", ImGuiDataType_U32, &m->unk_BC, NULL, NULL, "%08X", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Scale (X)", (float*)&m->scaleX, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Scale (Y)", (float*)&m->scaleY, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Scale (Z)", (float*)&m->scaleZ, "%.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Rotation (rad)", (float*)&m->rotation, "%.3f", ImGuiInputTextFlags_ReadOnly);
}

int main(int argc, char** argv){
    printf("MobyViewer built on %s at %s\n", __DATE__, __TIME__);

    GLFWwindow* window = NULL;
    int ret = init_OGL_ImGui(&window);
    glfwSwapInterval(1); //Enable V-Sync
    IM_ASSERT(ret);
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL; //Disable saving, as there are no window positions to save anyways.

    //Load icon
    GLFWimage winIcon = { 0 };
    winIcon.pixels = stbi_load_from_memory(icon, iconLen, &winIcon.width, &winIcon.height, NULL, 4 /* RGBA */);
    glfwSetWindowIcon(window, 1, &winIcon);
    stbi_image_free(winIcon.pixels);
    
    loadStrings();

    // Our state
    bool showDemoWindow = false;
    int currentTarget = TARGET_PLATFORM_PS2EMU;

    int filterClass = 0; bool doFilteringByClass = false;
    float maxDistanceToPlayer = 10.0f; bool doFilteringByDistance = false;
    bool disableDataUpdate = false;
    bool showVisibleMobysOnly = false;
    
#ifdef _WIN32
    PS2EmuTargetInitParams ps2trgtprm = {false, 0};
#else
    PS2EmuTargetInitParams ps2trgtprm = { true, 0 };
#endif
    //PS3EmuTargetInitParams
    //PS3TargetInitParams
    //PSVitaTargetInitParams
    void* initParams = nullptr;

    DBMgr* database = new DBMgr;
    if (database == nullptr) {
        fprintf(stderr, "[DBMgr] Failed to allocate database manager object!\n");
        return 0;
    }
    database->setNewTarget(currentTarget);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();



        if (!disableDataUpdate) {
            database->refresh();
        }

        uint32_t num_mobies = database->getMobysCount(), stackBase = database->getStackBase(), stackTop = database->getStackTop();
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);

            {
                ImGui::BeginMainMenuBar();
                char menuBarTextBuffer[64] = { 0 };
                if (num_mobies != 0) {
                    sprintf(menuBarTextBuffer, "Moby Table Base @ 0x%08X", stackBase);
                    if (ImGui::MenuItem(menuBarTextBuffer)) {
                        sprintf(menuBarTextBuffer, "0x%08X", stackBase);
                        ImGui::SetClipboardText(menuBarTextBuffer);
                    }
                    sprintf(menuBarTextBuffer, "Moby Table Top @ 0x%08X", stackTop);
                    if (ImGui::MenuItem(menuBarTextBuffer)) {
                        sprintf(menuBarTextBuffer, "0x%08X", stackTop);
                        ImGui::SetClipboardText(menuBarTextBuffer);
                    }
                    sprintf(menuBarTextBuffer, "Loaded mobies : %d", num_mobies);
                    ImGui::Text(menuBarTextBuffer);
                }
                sprintf(menuBarTextBuffer, "%.0f FPS ", io.Framerate);
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(menuBarTextBuffer).x);
                ImGui::Text(menuBarTextBuffer);
                float menubarheight = ImGui::GetWindowSize().y;
                ImGui::EndMainMenuBar();
                ImGui::SetNextWindowPos(ImVec2(0, menubarheight));
                ImGui::SetNextWindowSize(ImVec2(width, height - menubarheight));
            }
            ImGui::Begin("ContentWindow", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

            ImGui::Checkbox("[DEBUG] Show Demo Window", &showDemoWindow);
           
                {
                    constexpr char* windowName = "Options and filters";
                    ImGui::BeginChild(windowName, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 150), true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);
                    ImGui::BeginMenuBar();
                    ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(windowName).x) / 2.0f);
                    ImGui::Text(windowName);
                    ImGui::EndMenuBar();

                    ImGui::Checkbox("Disable updating", &disableDataUpdate); ImGui::SameLine();
                    ImGui::Checkbox("Only show visible mobys", &showVisibleMobysOnly);

                    if (ImGui::BeginTable("filters", 2, ImGuiTableFlags_SizingStretchProp)) {
                        ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() * 0.73f);
                        if (filterClass < 0) filterClass = 0;
                        if (filterClass > 65535) filterClass = 65535;
                        ImGui::InputInt("oClass", &filterClass);
                        if (ImGui::IsItemHovered()) {
                            char* oClassName = getOClassStringForID(filterClass, database->getCurrentGame());
                            if (oClassName != nullptr) ImGui::SetTooltip(oClassName);
                        }
                        ImGui::TableNextColumn(); ImGui::Checkbox("Enable##oClass", &doFilteringByClass); ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() * 0.73f);
                        ImGui::DragFloat("Max distance", &maxDistanceToPlayer); ImGui::TableNextColumn(); ImGui::Checkbox("Enable##dist", &doFilteringByDistance);
                        ImGui::EndTable();
                    }

                    ImGui::EndChild();
                }
                ImGui::SameLine();
                {
                    bool isTargetAttached = database->isTargetAttached();
                    constexpr char* windowName = "Target parameters";
                    char workBuf[64];
                    ImGui::BeginChild(windowName, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f - 10.0f, 150), true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
                    ImGui::BeginMenuBar();
                    ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(windowName).x) / 2.0f);
                    ImGui::Text(windowName);
                    ImGui::EndMenuBar();

                    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize("Target").x);
                    if (ImGui::BeginCombo("Target", TargetsNames[currentTarget])) {
                        for (int i = 0; i < TARGET_PLATFORMS_COUNT; i++) {
                            const bool isSelected = (i == currentTarget);
                            if (ImGui::Selectable(TargetsNames[i], isSelected)) {
                                if (i != currentTarget) {
                                    currentTarget = i;
                                    database->setNewTarget(currentTarget);
                                }
                            }
                            if (isSelected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    {
                        constexpr char* subWndName = "Target Management";
                        ImGui::BeginChild(subWndName, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 100), false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration);
                        ImGui::BeginMenuBar();
                        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(subWndName).x) / 2.0f);
                        ImGui::Text(subWndName);
                        ImGui::EndMenuBar();
                        sprintf(workBuf, "Status : %s", isTargetAttached ? "Attached" : "Not Attached");
                        ImGui::Text(workBuf);
                        sprintf(workBuf, "Current game : %s", getGameNameFromID(database->getCurrentGame()));
                        ImGui::Text(workBuf);
                        if (ImGui::Button(isTargetAttached ? "Reattach" : "Attach")) {
                            database->initializeTarget(initParams, isTargetAttached); //Force cleanup if already init
                            if (!isTargetAttached) database->refreshStateAndDB(); //Flush the DB once to initialize target, if we haven't init.
                        }
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip(isTargetAttached ? "Reconnect to target platform" : "Connect to target platform with specified parameters");
                        }

                        ImGui::SameLine();
                        if (isTargetAttached){
                            if (ImGui::Button("Detach"))
                                database->cleanupTarget(currentTarget);
                            if (ImGui::IsItemHovered())
                                ImGui::SetTooltip("Disconnect from target platform");
                        }


                        ImGui::SameLine();
                        if (ImGui::Button("Full refresh") && isTargetAttached)
                            database->refreshStateAndDB();
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip(isTargetAttached ? "Use this when i.e. changing planets on PS2Emu" : "You need to attach to a target first !");
                        ImGui::EndChild();
                    }

                    ImGui::SameLine();

                    {
                        constexpr char* subWndName = "Target Options";
                        ImGui::BeginChild(subWndName, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 100), false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);
                        ImGui::BeginMenuBar();
                        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(subWndName).x) / 2.0f);
                        ImGui::Text(subWndName);
                        ImGui::EndMenuBar();
                        switch (database->getCurrentTarget()) {
                        case TARGET_PLATFORM_PS2EMU:
                            ImGui::Checkbox("Use IPC", &ps2trgtprm.useIPC);
                            if (ImGui::IsItemHovered())
                                ImGui::SetTooltip("Usage of IPC is experimental and not recommended.\nIPC is mandatory on non-Win32 platforms.\n");

                            if (ps2trgtprm.useIPC) {
                                ImGui::InputScalar("IPC Port", ImGuiDataType_U32, &ps2trgtprm.port);
                                if (ImGui::IsItemHovered())
                                    ImGui::SetTooltip("Use 0 for default port.");
                                if (ps2trgtprm.port < 0) ps2trgtprm.port = 0;
                                if (ps2trgtprm.port > 65535) ps2trgtprm.port = 65535;
                            }
                            initParams = &ps2trgtprm;
                            break;

                        default:
                            ImGui::Text("No options for current target.");
                            initParams = nullptr;
                            break;
                        }
                        ImGui::EndChild();
                    }
                    ImGui::EndChild();
                }

          
           


            uint32_t displayedCount = 0, currentGame = database->getCurrentGame();
            Moby* playerMoby = database->getMobyPointer(0); //Ratchet moby is always the first (?)
            for (int i = 0; i < num_mobies ; i++) {
                Moby* mb = database->getMobyPointer(i);
                if (mb == nullptr) {
                    fprintf(stderr, "[main] - getting moby %d returned nullptr.\n", i); continue;
                }
                if ((doFilteringByClass && mb->oClass != filterClass) ||
                    (doFilteringByDistance && DISTANCE(mb, playerMoby) > maxDistanceToPlayer) ||
                    (showVisibleMobysOnly && !mb->visible)) 
                    continue; //Skip moby if it doesn't respect active filters

                char winName[64];
                sprintf(winName, "Moby #%d @ 0x%08X", i, stackBase + 0x100 * i);
                if (mb->visible)
                    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(86.0f / 255.0f, 47.0f / 255.0f, 9.0f / 255.0f, 1.0f));
                ImGui::BeginChild(winName, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.33f, 225), true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);
                ImGui::BeginMenuBar();
                if (ImGui::MenuItem(winName)) {
                    sprintf(winName, "0x%08X", stackBase + 0x100 * i);
                    ImGui::SetClipboardText(winName);
                }
                if (mb->visible)
                    ImGui::PopStyleColor();
                
                
                ImGui::EndMenuBar();
                AddMobyWidget(mb, currentGame);
                ImGui::EndChild();
                displayedCount++;
                if ( (displayedCount) % 3 != 0) ImGui::SameLine();
            }
            

            ImGui::End();
        }

        if (showDemoWindow)
            ImGui::ShowDemoWindow();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for (int i = 0; i < TARGET_PLATFORMS_COUNT; i++) {
        database->cleanupTarget(i);
    }
    delete database;

    unloadStrings();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}