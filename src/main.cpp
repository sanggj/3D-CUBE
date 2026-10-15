#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <thread>
#include <vector>

#include "config.h"
#include "mat4.h"
#include "cube_geometry.h"
#include "shaders.h"
#include "shader.h"
#include "instances.h"
#include "msaa.h"
#include "input.h"
#include "ui.h"

static void uploadInstances(GLuint instVbo, const std::vector<Instance>& data) {
    glBindBuffer(GL_ARRAY_BUFFER, instVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        data.size() * sizeof(Instance),
        data.data(),
        GL_DYNAMIC_DRAW);
}

static char* envDup(const char* name) {
#ifdef _MSC_VER
    char* v = nullptr;
    size_t len = 0;

    if (_dupenv_s(&v, &len, name) != 0)
        return nullptr;

    return v;
#else
    const char* v = std::getenv(name);

    if (!v)
        return nullptr;

    char* copy = (char*)std::malloc(std::strlen(v) + 1);

    if (copy)
        std::strcpy(copy, v);

    return copy;
#endif
}

static std::vector<Instance> generateInstances(int n) {
    if (useSeedOverride)
        return buildInstancesSeeded(n, masterSeed);

    return buildInstances(n);
}

static void handleRebuild(GLuint instVbo, std::vector<Instance>& instances) {
    if (INST < 1)
        INST = 1;

    try {
        double t0 = glfwGetTime();
        std::vector<Instance> next = generateInstances(INST);
        double t1 = glfwGetTime();

        instances.swap(next);

        uploadInstances(instVbo, instances);

        std::fprintf(
            stderr,
            "rebuild inst=%d build_ms=%.1f upload_ms=%.1f\n",
            INST,
            (t1 - t0) * 1000.0,
            (glfwGetTime() - t1) * 1000.0);

        std::snprintf(cubeCountBuffer, sizeof(cubeCountBuffer), "%d", INST);
        titleDirty = true;
    } catch (const std::bad_alloc&) {
        std::fprintf(stderr, "OUT OF MEMORY\n");

        INST = 100000;

        std::snprintf(cubeCountBuffer, sizeof(cubeCountBuffer), "%d", INST);

        instances = generateInstances(INST);
        uploadInstances(instVbo, instances);

        titleDirty = true;
    }

    rebuildInstances = false;
}

int main() {
    glfwSetErrorCallback([](int code, const char* desc) {
        std::fprintf(stderr, "GLFW error %d: %s\n", code, desc);
    });

    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 0);

    GLFWwindow* win = glfwCreateWindow(W, H, "CubeChaos", nullptr, nullptr);

    if (!win) {
        std::fprintf(
            stderr, "Failed to create GLFW window (OpenGL 4.6 core required)\n");
        return 1;
    }

    glfwMakeContextCurrent(win);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to load OpenGL functions via glad\n");
        return 1;
    }

    std::fprintf(
        stderr,
        "OpenGL %s | GPU: %s\n",
        (const char*)glGetString(GL_VERSION),
        (const char*)glGetString(GL_RENDERER));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(win, false);
    ImGui_ImplOpenGL3_Init("#version 460");

    glEnable(GL_DEPTH_TEST);

    GLuint prog = linkProgram(
        compileShader(GL_VERTEX_SHADER, VERTEX_SHADER_SRC),
        compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SRC));

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint instVbo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVerts), kCubeVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(kCubeIndices),
        kCubeIndices,
        GL_STATIC_DRAW);

    glGenBuffers(1, &instVbo);

    if (char* seedEnv = envDup("CUBECHAOS_SEED")) {
        char* endp = nullptr;
        unsigned long long v = std::strtoull(seedEnv, &endp, 10);

        if (!endp || endp == seedEnv || *endp != '\0') {
            std::fprintf(
                stderr, "CUBECHAOS_SEED invalid (%s), ignored\n", seedEnv);
        } else {
            std::uint32_t k = (std::uint32_t)v;

            masterSeed = {
                k,
                k * 2654435761u,
                k >> 13,
                0x9E3779B9u
            };

            useSeedOverride = true;

            std::fprintf(
                stderr,
                "CUBECHAOS_SEED=%s (deterministic layout, debug only)\n",
                seedEnv);
        }

        std::free(seedEnv);
    }

    double genT0 = glfwGetTime();
    std::vector<Instance> instances = generateInstances(INST);
    unsigned hc = std::thread::hardware_concurrency();
    std::fprintf(
        stderr,
        "startup build inst=%d threads=%u ms=%.1f\n",
        INST,
        hc ? hc : 1u,
        (glfwGetTime() - genT0) * 1000.0);
    uploadInstances(instVbo, instances);

    glVertexAttribPointer(
        1, 4, GL_FLOAT, GL_FALSE, sizeof(Instance), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    GLint mvpLoc = glGetUniformLocation(prog, "mvp");
    GLint timeLoc = glGetUniformLocation(prog, "time");

    MsaaTarget msaa;

    GLuint gpuQ[2][2] = {};
    glGenQueries(2, gpuQ[0]);
    glGenQueries(2, gpuQ[1]);

    int gpuSlot = 0;
    bool gpuInFlight[2] = {};

    installInputCallbacks(win);

    lastTime = glfwGetTime();

    int fbW = 0;
    int fbH = 0;

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        pollInput(win);

        glfwGetFramebufferSize(win, &fbW, &fbH);

        if (fbW <= 0 || fbH <= 0) {
            lastTime = glfwGetTime();
            glfwWaitEvents();
            continue;
        }

        if (rebuildInstances)
            handleRebuild(instVbo, instances);

        double now = glfwGetTime();
        double dt = now - lastTime;
        lastTime = now;

        fpsAcc += dt;
        fpsCount++;
        frameMs += (dt * 1000.0 - frameMs) * 0.05;
        pushFrameTime(dt * 1000.0);

        if (fpsAcc >= 1.0) {
            fps = fpsCount / fpsAcc;
            fpsAcc = 0.0;
            fpsCount = 0;
            updateFrameStats();
            titleDirty = true;
        }

        if (uiVisible) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            buildUi(win);
        }

        static int cachedCulling = -1;
        static int cachedWireframe = -1;

        int cullNow = enableCulling ? 1 : 0;
        int wireNow = wireframe ? 1 : 0;

        if (cullNow != cachedCulling) {
            if (cullNow)
                glEnable(GL_CULL_FACE);
            else
                glDisable(GL_CULL_FACE);
            cachedCulling = cullNow;
        }

        if (wireNow != cachedWireframe) {
            glPolygonMode(
                GL_FRONT_AND_BACK, wireNow ? GL_LINE : GL_FILL);
            cachedWireframe = wireNow;
        }

        msaa.ensure(fbW, fbH, aaSamples);

        glBindFramebuffer(GL_FRAMEBUFFER, msaa.fbo ? msaa.fbo : 0);

        bool gpuIssued = false;

        if (gpuInFlight[gpuSlot]) {
            GLuint64 readyA = 0;
            GLuint64 readyB = 0;

            glGetQueryObjectui64v(gpuQ[gpuSlot][0], GL_QUERY_RESULT_AVAILABLE, &readyA);
            glGetQueryObjectui64v(gpuQ[gpuSlot][1], GL_QUERY_RESULT_AVAILABLE, &readyB);

            if (readyA && readyB) {
                GLuint64 g0 = 0;
                GLuint64 g1 = 0;

                glGetQueryObjectui64v(gpuQ[gpuSlot][0], GL_QUERY_RESULT, &g0);
                glGetQueryObjectui64v(gpuQ[gpuSlot][1], GL_QUERY_RESULT, &g1);

                gpuMs = (double)(g1 - g0) / 1.0e6;

                gpuInFlight[gpuSlot] = false;
            }
        }

        if (!gpuInFlight[gpuSlot]) {
            glQueryCounter(gpuQ[gpuSlot][0], GL_TIMESTAMP);
            gpuIssued = true;
        }

        glViewport(0, 0, fbW, fbH);

        glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Mat4 P = perspective(FOV, (float)fbW / (float)fbH, NEAR_Z, FAR_Z);
        Mat4 V = translate(0.0f, 0.0f, -getDist());
        Mat4 M = mul(rotY((float)yaw), rotX((float)pitch));
        Mat4 MVP = mul(P, mul(V, M));

        glUseProgram(prog);
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, MVP.m);
        glUniform1f(timeLoc, (float)now);

        glBindVertexArray(vao);

        glDrawElementsInstanced(
            GL_TRIANGLES,
            (GLsizei)(sizeof(kCubeIndices) / sizeof(kCubeIndices[0])),
            GL_UNSIGNED_INT,
            nullptr,
            INST);

        if (msaa.fbo) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa.fbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(
                0, 0, fbW, fbH,
                0, 0, fbW, fbH,
                GL_COLOR_BUFFER_BIT,
                GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        if (gpuIssued) {
            glQueryCounter(gpuQ[gpuSlot][1], GL_TIMESTAMP);
            gpuInFlight[gpuSlot] = true;
            gpuSlot = 1 - gpuSlot;
        }

        if (uiVisible) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        if (titleDirty) {
            char title[256];

            if (uiVisible)
                std::snprintf(
                    title, sizeof(title), "FPS: %.1f | INST: %d", fps, INST);
            else
                std::snprintf(
                    title,
                    sizeof(title),
                    "FPS: %.1f | INST: %d | UI hidden - press H",
                    fps,
                    INST);

            glfwSetWindowTitle(win, title);
            titleDirty = false;
        }

        glfwSwapBuffers(win);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    msaa.destroy();

    glDeleteQueries(2, gpuQ[0]);
    glDeleteQueries(2, gpuQ[1]);

    glDeleteBuffers(1, &instVbo);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(prog);

    glfwDestroyWindow(win);
    glfwTerminate();

    return 0;
}
