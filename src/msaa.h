#pragma once

#include <glad/glad.h>

struct MsaaTarget {
    GLuint fbo = 0;
    GLuint colorRb = 0;
    GLuint depthRb = 0;

    int w = 0;
    int h = 0;
    int s = 0;

    void ensure(int width, int height, int samples);
    void destroy();
};
