#include "shader.h"

#include <cstdio>
#include <cstdlib>

GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);

    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);

    if (!ok) {
        char log[8192];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        std::fprintf(stderr, "%s\n", log);
        std::exit(1);
    }

    return s;
}

GLuint linkProgram(GLuint vs, GLuint fs) {
    GLuint p = glCreateProgram();

    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);

    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);

    if (!ok) {
        char log[8192];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        std::fprintf(stderr, "%s\n", log);
        std::exit(1);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return p;
}
