#include "msaa.h"
#include "config.h"

#include <cstdio>

void MsaaTarget::destroy() {
    if (!fbo)
        return;

    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &colorRb);
    glDeleteRenderbuffers(1, &depthRb);

    fbo = 0;
    colorRb = 0;
    depthRb = 0;
}

void MsaaTarget::ensure(int width, int height, int samples) {
    if (samples <= 0) {
        destroy();
        return;
    }

    if (fbo && width == w && height == h && samples == s)
        return;

    destroy();

    w = width;
    h = height;
    s = samples;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffers(1, &colorRb);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRb);
    glRenderbufferStorageMultisample(
        GL_RENDERBUFFER, s, GL_RGBA8, w, h);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRb);

    glGenRenderbuffers(1, &depthRb);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRb);
    glRenderbufferStorageMultisample(
        GL_RENDERBUFFER, s, GL_DEPTH_COMPONENT24, w, h);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRb);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
        GL_FRAMEBUFFER_COMPLETE) {
        std::fprintf(stderr, "MSAA FBO incomplete, falling back to no AA\n");
        destroy();
        aaSamples = 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
