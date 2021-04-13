#ifndef INIT_H
#define INIT_H

#include <glad/glad.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>


#define WINDOW_TITLE "Moby Viewer"
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

//Returns 1 on success, 0 on error.
bool init_OGL_ImGui(GLFWwindow** window);

#endif