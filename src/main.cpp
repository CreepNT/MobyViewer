#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cctype>

#include "targets.h"
#include "ogl_imgui.h"
#include "widgets.h"
#include "dbmgr.h"
#include "icon.h"
#include "moby.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


char tmpBuf[0x400] = { 0 }; //Buffer used for any operation that requires some room
#define stmp_sprintf(fmt, ...) snprintf(tmpBuf, sizeof(tmpBuf), fmt, ## __VA_ARGS__) //Secure TeMPorary buffer sprintf

#ifdef _WIN32
    static PS2EmuTargetInitParams ps2trgtprm = { false, 0 };
#else
    static PS2EmuTargetInitParams ps2trgtprm = { true, 0 };
#endif
//PS3EmuTargetInitParams
//PS3TargetInitParams
//PSVitaTargetInitParams

static int currentTarget = TARGET_PLATFORM_PS2EMU;
static void* initParams = nullptr;
static DBMgr* database = nullptr;

float DISTANCE(Moby* a, Moby* b) {
    if (!a || !b) return 100.0f;
    return sqrtf(powf(b->pos.x - a->pos.x, 2.0f) + powf(b->pos.y - a->pos.y, 2.0f) + powf(b->pos.z - a->pos.z, 2.0f));
}

int main(int argc, char** argv){
    printf("MobyViewer built on %s at %s\n", __DATE__, __TIME__);

    GLFWwindow* window = NULL;
    int ret = init_OGL_ImGui(&window);
    glfwSwapInterval(1);
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

    int filterClass = 0; bool doFilteringByClass = false;
    float maxDistanceToPlayer = 10.0f; bool doFilteringByDistance = false;
    bool disableDataUpdate = false;
    bool hideNonDrawnMobys = false;
    


    database = new DBMgr;
    if (database == nullptr) {
        fprintf(stderr, "[DBMgr] Failed to allocate database manager object!\n");
        return EXIT_FAILURE;
    }
    database->setNewTarget(currentTarget);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        if (!disableDataUpdate) {
            if (!database->refresh()) { //Detach
                printf("[Main] Failed refresh - detaching!\n");
                database->cleanupTarget();
            }
        }

        uint32_t num_mobies = database->getMobysCount(), stackBase = database->getStackBase(), stackTop = database->getStackTop();
        currentTarget = database->getCurrentTarget();
        {
            //Draw menu bar
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            {
                ImGui::BeginMainMenuBar();
                if (num_mobies != 0) {
                    stmp_sprintf("Moby Table Base @ 0x%08X", stackBase);
                    if (ImGui::MenuItem(tmpBuf)) {
                        stmp_sprintf("0x%08X", stackBase);
                        ImGui::SetClipboardText(tmpBuf);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Click to copy address");

                    stmp_sprintf("Moby Table Top @ 0x%08X", stackTop);
                    if (ImGui::MenuItem(tmpBuf)) {
                        stmp_sprintf("0x%08X", stackTop);
                        ImGui::SetClipboardText(tmpBuf);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Click to copy address");

                    stmp_sprintf("Alive mobys : %d", num_mobies);
                    ImGui::Text(tmpBuf);
                }
                stmp_sprintf("%.0f FPS ", io.Framerate);
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(tmpBuf).x);
                ImGui::Text(tmpBuf);
                float menubarheight = ImGui::GetWindowSize().y;
                ImGui::EndMainMenuBar();
                ImGui::SetNextWindowPos(ImVec2(0, menubarheight));
                ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width), static_cast<float>(height) - menubarheight));
            }

            //Start fullcanvas window
            ImGui::Begin("ContentWindow", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

            ImGui::Checkbox("[DEBUG] Show Demo Window", &showDemoWindow);

            GameID currentGame = database->getCurrentGame();

            //Draw "Options and filters" subwindow
            {
                constexpr char* windowName = "Options and filters";
                ImGui::BeginChild(windowName, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 150), true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);
                ImGui::BeginMenuBar();
                ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(windowName).x) / 2.0f);
                ImGui::Text(windowName);
                ImGui::EndMenuBar();

                ImGui::Checkbox("Freeze data", &disableDataUpdate); ImGui::SameLine();
                ImGui::Checkbox("Hide non-drawn mobys", &hideNonDrawnMobys);

                if (ImGui::BeginTable("filters", 2, ImGuiTableFlags_SizingStretchProp)) {
                    ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() * 0.73f);
                    if (filterClass < 0) filterClass = 0;
                    if (filterClass > 65535) filterClass = 65535;
                    ImGui::InputInt("oClass", &filterClass);
                    if (ImGui::IsItemHovered()) {
                        char const* oClassName = GetOClassStringForID(filterClass, database->getCurrentGame());
                        if (oClassName != nullptr) ImGui::SetTooltip(oClassName);
                    }
                    ImGui::TableNextColumn(); ImGui::Checkbox("Enable##oClass", &doFilteringByClass); ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() * 0.73f);
                    if (currentGame == GAME_RC4) {
                        doFilteringByDistance = false;
                    }
                    else {
                        ImGui::DragFloat("Max distance", &maxDistanceToPlayer);
                        ImGui::TableNextColumn(); ImGui::Checkbox("Enable##dist", &doFilteringByDistance);
                    }
                    ImGui::EndTable();
                }

                ImGui::EndChild();
            }
            ImGui::SameLine();

            //Draw "Target parameters" subwindow
            {
                constexpr char* windowName = "Target parameters";
                const bool isTargetAttached = database->isTargetAttached();

                ImGui::BeginChild(windowName, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f - 10.0f, 150), true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
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

                //Target management
                {
                    constexpr char* subWndName = "Target Management";
                    ImGui::BeginChild(subWndName, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 100), false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration);
                    ImGui::BeginMenuBar();
                    ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(subWndName).x) / 2.0f);
                    ImGui::Text(subWndName);
                    ImGui::EndMenuBar();
                    if (isTargetAttached) ImGui::Text("Status : Attached");
                    else ImGui::Text("Status : Not Attached");
                    stmp_sprintf("Current game : %s", GetGameNameFromID(database->getCurrentGame()));
                    ImGui::Text(tmpBuf);

                    if (ImGui::Button(isTargetAttached ? "Reattach" : "Attach")) {
                        database->initializeTarget(initParams, isTargetAttached); //Force cleanup if already init
                        if (!isTargetAttached) database->refreshStateAndDB(); //Flush the DB once to initialize target, if we haven't init.
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(isTargetAttached ? "Reconnect to target platform" : "Connect to target platform");
                    }

                    ImGui::SameLine();
                    if (isTargetAttached) {
                        if (ImGui::Button("Detach"))
                            database->cleanupTarget(currentTarget);
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Disconnect from target platform");
                    }


                    ImGui::SameLine();
                    if (ImGui::Button("Full refresh") && isTargetAttached)
                        if (!database->refreshStateAndDB()) {
                            printf("[Main] Failed full refresh - detaching.\n");
                            database->cleanupTarget(database->getCurrentTarget()); //Detach
                        }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(isTargetAttached ? "Renew all target state (i.e. all moby stack pointers)" : "You need to attach to a target first !");
                    ImGui::EndChild();
                }

                ImGui::SameLine();

                //Target Options
                {
                    constexpr char* subWndName = "Target Options";
                    ImGui::BeginChild(subWndName, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 100), false, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);
                    ImGui::BeginMenuBar();
                    ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(subWndName).x) / 2.0f);
                    ImGui::Text(subWndName);
                    ImGui::EndMenuBar();
                    switch (currentTarget) {
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

            //Draw Mobys data
            {
                uint32_t displayedCount = 0;
                if (currentGame != GAME_RC4) {
                    Moby* playerMoby = (Moby*)database->getMobyPointer(0); //Player moby should always be the first
                    for (unsigned int i = 0; i < num_mobies; i++) {
                        Moby* mb = static_cast<Moby*>(database->getMobyPointer(i));
                        uintptr_t mbAddr = database->getMobyAddress(i);
                        if ((doFilteringByClass && mb->oClass != filterClass) ||
                            (doFilteringByDistance && DISTANCE(mb, playerMoby) > maxDistanceToPlayer) ||
                            (hideNonDrawnMobys && !mb->drawn))
                            continue; //Skip moby if it doesn't respect active filters

                        
                        stmp_sprintf("Moby #%u @ 0x%08llX", i, (uint64_t)mbAddr);
                        if (mb->drawn)
                            ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(86.0f / 255.0f, 47.0f / 255.0f, 9.0f / 255.0f, 1.0f));

                        ImGui::BeginChild(tmpBuf, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.33f, 225), true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);
                        ImGui::BeginMenuBar();
                        if (ImGui::MenuItem(tmpBuf)) {
                            stmp_sprintf("0x%08llX", (uint64_t)mbAddr);
                            ImGui::SetClipboardText(tmpBuf);
                        }
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Click to copy address");
                        
                        if (mb->drawn)
                            ImGui::PopStyleColor();

                        ImGui::EndMenuBar();

                        AddOG3MobyWidgets(mb, currentGame);
                        ImGui::EndChild();
                        displayedCount++;
                        if ((displayedCount) % 3 != 0) ImGui::SameLine();
                    }
                }
                else {
                    for (unsigned int i = 0; i < num_mobies; i++) {
                        RC4Moby* mb = static_cast<RC4Moby*>(database->getMobyPointer(i));
                        uintptr_t mbAddr = database->getMobyAddress(i);

                        if ((doFilteringByClass && mb->oClass != filterClass) ||
                            (hideNonDrawnMobys && !mb->drawn))
                            continue; //Skip moby if it doesn't respect active filters

                        
                        stmp_sprintf("Moby #%u @ 0x%08llX", i, (uint64_t)mbAddr);
                        if (mb->drawn)
                            ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(86.0f / 255.0f, 47.0f / 255.0f, 9.0f / 255.0f, 1.0f));
                        ImGui::BeginChild(tmpBuf, ImVec2(ImGui::GetWindowContentRegionWidth() * 0.33f, 225), true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);
                        ImGui::BeginMenuBar();
                        if (ImGui::MenuItem(tmpBuf)) {
                            stmp_sprintf("0x%08llX", (uint64_t)mbAddr);
                            ImGui::SetClipboardText(tmpBuf);
                        }
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Click to copy address");

                        if (mb->drawn)
                            ImGui::PopStyleColor();

                        ImGui::EndMenuBar();

                        AddRC4MobyWidgets(mb);
                        ImGui::EndChild();
                        displayedCount++;
                        if ((displayedCount) % 3 != 0) ImGui::SameLine();
                    }
                }


                ImGui::End();
            }
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