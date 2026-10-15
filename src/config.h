#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

inline int W = 1280;
inline int H = 720;

inline int INST = 34356;

inline float SPACING = 4.0f;

inline float ROT_SENS = 0.01f;

inline float ZOOM_BASE = 120.0f;
inline float ZOOM_SPEED = 0.15f;

inline float FOV = 70.0f;

inline float NEAR_Z = 0.1f;
inline float FAR_Z = 10000.0f;

inline bool enableCulling = true;
inline bool wireframe = false;

inline int aaSamples = 4;
inline bool vsync = false;

inline int MAX_INSTANCES = 20000000;

inline double yaw = 0.0;
inline double pitch = 0.0;

inline bool drag = false;

inline double lx = 0.0;
inline double ly = 0.0;

inline double zoomExp = 0.0;

inline double fps = 0.0;
inline double fpsAcc = 0.0;

inline int fpsCount = 0;

inline double lastTime = 0.0;

inline double frameMs = 0.0;

inline bool titleDirty = true;

inline bool uiVisible = true;

inline bool rebuildInstances = false;

inline char cubeCountBuffer[64] = "34356";

inline constexpr int kFrameHist = 512;

inline double frameHistMs[kFrameHist] = {};
inline int frameHistCount = 0;
inline int frameHistIdx = 0;

inline double ftP50Ms = 0.0;
inline double ftP95Ms = 0.0;
inline double ftP99Ms = 0.0;
inline double ftMaxMs = 0.0;

inline double gpuMs = 0.0;

inline bool useSeedOverride = false;

inline std::array<std::uint32_t, 4> masterSeed = {};

inline void pushFrameTime(double ms) {
    frameHistMs[frameHistIdx] = ms;
    frameHistIdx = (frameHistIdx + 1) % kFrameHist;
    if (frameHistCount < kFrameHist)
        frameHistCount++;
}

inline void updateFrameStats() {
    if (frameHistCount == 0)
        return;

    double sorted[kFrameHist];
    std::memcpy(sorted, frameHistMs, frameHistCount * sizeof(double));
    std::sort(sorted, sorted + frameHistCount);

    ftP50Ms = sorted[frameHistCount / 2];
    ftP95Ms = sorted[(int)((frameHistCount - 1) * 0.95)];
    ftP99Ms = sorted[(int)((frameHistCount - 1) * 0.99)];
    ftMaxMs = sorted[frameHistCount - 1];
}

inline float getDist() {
    return (float)(ZOOM_BASE * std::exp(zoomExp));
}
