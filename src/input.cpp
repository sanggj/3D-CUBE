#include "input.h"
#include "config.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"

static void mouseButtonCallback(
    GLFWwindow* window, int button, int action, int mods) {

    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    ImGuiIO& io = ImGui::GetIO();

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        drag = false;

    if (uiVisible && io.WantCaptureMouse)
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        drag = true;
        glfwGetCursorPos(window, &lx, &ly);
    }
}

static void cursorPosCallback(GLFWwindow* window, double x, double y) {
    ImGui_ImplGlfw_CursorPosCallback(window, x, y);

    ImGuiIO& io = ImGui::GetIO();

    if (uiVisible && io.WantCaptureMouse) {
        lx = x;
        ly = y;
        return;
    }

    if (!drag)
        return;

    double dx = x - lx;
    double dy = y - ly;

    lx = x;
    ly = y;

    yaw += dx * ROT_SENS;
    pitch += dy * ROT_SENS;

    if (pitch > 1.55)
        pitch = 1.55;

    if (pitch < -1.55)
        pitch = -1.55;
}

static void scrollCallback(GLFWwindow* window, double xoff, double yoff) {
    ImGui_ImplGlfw_ScrollCallback(window, xoff, yoff);

    ImGuiIO& io = ImGui::GetIO();

    if (uiVisible && io.WantCaptureMouse)
        return;

    zoomExp -= yoff * ZOOM_SPEED;

    if (zoomExp < -5.0)
        zoomExp = -5.0;

    if (zoomExp > 3.7)
        zoomExp = 3.7;
}

static void keyCallback(
    GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_H && action == GLFW_PRESS)
        uiVisible = !uiVisible;

    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}

static void charCallback(GLFWwindow* window, unsigned int c) {
    ImGui_ImplGlfw_CharCallback(window, c);
}

void installInputCallbacks(GLFWwindow* win) {
    glfwSetMouseButtonCallback(win, mouseButtonCallback);
    glfwSetCursorPosCallback(win, cursorPosCallback);
    glfwSetScrollCallback(win, scrollCallback);
    glfwSetKeyCallback(win, keyCallback);
    glfwSetCharCallback(win, charCallback);
}

void pollInput(GLFWwindow* win) {
    bool held =
        glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (!held) {
        drag = false;
        ImGui::GetIO().MouseDown[0] = false;
    }
}
