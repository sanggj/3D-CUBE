#include "ui.h"
#include "config.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"

#include <cstdlib>
#include <thread>

static void setAaSamples(int s) {
    GLint maxSamples = 4;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

    aaSamples = s < (int)maxSamples ? s : (int)maxSamples;
}

void buildUi(GLFWwindow* win) {
    ImGui::SetNextWindowSize(ImVec2(430, 0), ImGuiCond_Once);

    ImGui::Begin("Settings");

    ImGui::PushItemWidth(220);

    ImGui::InputText(
        "Cube Count",
        cubeCountBuffer,
        sizeof(cubeCountBuffer),
        ImGuiInputTextFlags_CharsDecimal);

    if (ImGui::IsItemDeactivatedAfterEdit()) {
        int newCount = std::atoi(cubeCountBuffer);

        if (newCount < 1)
            newCount = 1;

        if (newCount > MAX_INSTANCES)
            newCount = MAX_INSTANCES;

        if (newCount != INST) {
            INST = newCount;
            rebuildInstances = true;
        }
    }

    ImGui::SliderFloat("Spacing", &SPACING, 1.0f, 20.0f);

    if (ImGui::IsItemDeactivatedAfterEdit())
        rebuildInstances = true;

    ImGui::Checkbox("Backface Culling", &enableCulling);
    ImGui::Checkbox("Wireframe", &wireframe);

    if (ImGui::Checkbox("VSync", &vsync))
        glfwSwapInterval(vsync ? 1 : 0);

    if (ImGui::Button("Reset Camera")) {
        yaw = 0.0;
        pitch = 0.0;
        zoomExp = 0.0;
    }

    ImGui::Separator();

    ImGui::Text("AA Samples: %d", aaSamples);

    if (ImGui::Button("AA OFF"))
        setAaSamples(0);

    ImGui::SameLine();

    if (ImGui::Button("2X"))
        setAaSamples(2);

    ImGui::SameLine();

    if (ImGui::Button("4X"))
        setAaSamples(4);

    ImGui::SameLine();

    if (ImGui::Button("8X"))
        setAaSamples(8);

    ImGui::Separator();

    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame: %.2f ms", frameMs);
    ImGui::Text("Frame p50: %.2f  p99: %.2f  max: %.2f",
                ftP50Ms, ftP99Ms, ftMaxMs);
    ImGui::Text("Instances: %d", INST);
    ImGui::Text("CPU Threads: %u", std::thread::hardware_concurrency());
    ImGui::Text("Change with caution!!");

    ImGui::PopItemWidth();

    ImGui::End();
}
