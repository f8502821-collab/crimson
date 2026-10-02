#pragma once
// Minimal GL 3.3 core function loader for the post-FX pass.
// MC ships a 3.2+ core context; we bind only what the pass needs.
// windows.h must come before GL/gl.h (WINGDIAPI/wglGetProcAddress).
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <GL/gl.h>
#include <cstdint>

namespace gl3 {

struct Api {
    void (APIENTRY *glGenVertexArrays)(int, unsigned*) = nullptr;
    void (APIENTRY *glBindVertexArray)(unsigned) = nullptr;
    void (APIENTRY *glGenBuffers)(int, unsigned*) = nullptr;
    void (APIENTRY *glBindBuffer)(unsigned, unsigned) = nullptr;
    void (APIENTRY *glBufferData)(unsigned, intptr_t, const void*, unsigned) = nullptr;
    void (APIENTRY *glVertexAttribPointer)(unsigned, int, unsigned, unsigned char, int, const void*) = nullptr;
    void (APIENTRY *glEnableVertexAttribArray)(unsigned) = nullptr;
    void (APIENTRY *glDrawArrays)(unsigned, int, int) = nullptr;
    unsigned (APIENTRY *glCreateShader)(unsigned) = nullptr;
    void (APIENTRY *glShaderSource)(unsigned, int, const char**, const int*) = nullptr;
    void (APIENTRY *glCompileShader)(unsigned) = nullptr;
    void (APIENTRY *glGetShaderiv)(unsigned, unsigned, int*) = nullptr;
    void (APIENTRY *glGetShaderInfoLog)(unsigned, int, int*, char*) = nullptr;
    unsigned (APIENTRY *glCreateProgram)() = nullptr;
    void (APIENTRY *glAttachShader)(unsigned, unsigned) = nullptr;
    void (APIENTRY *glLinkProgram)(unsigned) = nullptr;
    void (APIENTRY *glGetProgramiv)(unsigned, unsigned, int*) = nullptr;
    void (APIENTRY *glGetProgramInfoLog)(unsigned, int, int*, char*) = nullptr;
    void (APIENTRY *glUseProgram)(unsigned) = nullptr;
    void (APIENTRY *glDeleteShader)(unsigned) = nullptr;
    int  (APIENTRY *glGetUniformLocation)(unsigned, const char*) = nullptr;
    void (APIENTRY *glUniform1f)(int, float) = nullptr;
    void (APIENTRY *glUniform2f)(int, float, float) = nullptr;
    void (APIENTRY *glUniform1i)(int, int) = nullptr;
};

inline Api api;
inline bool g_ok = false;

inline bool bind() {
    api.glGenVertexArrays      = reinterpret_cast<decltype(api.glGenVertexArrays)>(wglGetProcAddress("glGenVertexArrays"));
    api.glBindVertexArray      = reinterpret_cast<decltype(api.glBindVertexArray)>(wglGetProcAddress("glBindVertexArray"));
    api.glGenBuffers           = reinterpret_cast<decltype(api.glGenBuffers)>(wglGetProcAddress("glGenBuffers"));
    api.glBindBuffer           = reinterpret_cast<decltype(api.glBindBuffer)>(wglGetProcAddress("glBindBuffer"));
    api.glBufferData           = reinterpret_cast<decltype(api.glBufferData)>(wglGetProcAddress("glBufferData"));
    api.glVertexAttribPointer  = reinterpret_cast<decltype(api.glVertexAttribPointer)>(wglGetProcAddress("glVertexAttribPointer"));
    api.glEnableVertexAttribArray = reinterpret_cast<decltype(api.glEnableVertexAttribArray)>(wglGetProcAddress("glEnableVertexAttribArray"));
    api.glDrawArrays           = reinterpret_cast<decltype(api.glDrawArrays)>(wglGetProcAddress("glDrawArrays"));
    api.glCreateShader         = reinterpret_cast<decltype(api.glCreateShader)>(wglGetProcAddress("glCreateShader"));
    api.glShaderSource         = reinterpret_cast<decltype(api.glShaderSource)>(wglGetProcAddress("glShaderSource"));
    api.glCompileShader        = reinterpret_cast<decltype(api.glCompileShader)>(wglGetProcAddress("glCompileShader"));
    api.glGetShaderiv          = reinterpret_cast<decltype(api.glGetShaderiv)>(wglGetProcAddress("glGetShaderiv"));
    api.glGetShaderInfoLog     = reinterpret_cast<decltype(api.glGetShaderInfoLog)>(wglGetProcAddress("glGetShaderInfoLog"));
    api.glCreateProgram        = reinterpret_cast<decltype(api.glCreateProgram)>(wglGetProcAddress("glCreateProgram"));
    api.glAttachShader         = reinterpret_cast<decltype(api.glAttachShader)>(wglGetProcAddress("glAttachShader"));
    api.glLinkProgram          = reinterpret_cast<decltype(api.glLinkProgram)>(wglGetProcAddress("glLinkProgram"));
    api.glGetProgramiv         = reinterpret_cast<decltype(api.glGetProgramiv)>(wglGetProcAddress("glGetProgramiv"));
    api.glGetProgramInfoLog    = reinterpret_cast<decltype(api.glGetProgramInfoLog)>(wglGetProcAddress("glGetProgramInfoLog"));
    api.glUseProgram           = reinterpret_cast<decltype(api.glUseProgram)>(wglGetProcAddress("glUseProgram"));
    api.glDeleteShader         = reinterpret_cast<decltype(api.glDeleteShader)>(wglGetProcAddress("glDeleteShader"));
    api.glGetUniformLocation   = reinterpret_cast<decltype(api.glGetUniformLocation)>(wglGetProcAddress("glGetUniformLocation"));
    api.glUniform1f            = reinterpret_cast<decltype(api.glUniform1f)>(wglGetProcAddress("glUniform1f"));
    api.glUniform2f            = reinterpret_cast<decltype(api.glUniform2f)>(wglGetProcAddress("glUniform2f"));
    api.glUniform1i            = reinterpret_cast<decltype(api.glUniform1i)>(wglGetProcAddress("glUniform1i"));

    g_ok = api.glGenVertexArrays && api.glBindVertexArray && api.glGenBuffers &&
           api.glBindBuffer && api.glBufferData && api.glVertexAttribPointer &&
           api.glEnableVertexAttribArray && api.glDrawArrays && api.glCreateShader &&
           api.glShaderSource && api.glCompileShader && api.glGetShaderiv &&
           api.glCreateProgram && api.glAttachShader && api.glLinkProgram &&
           api.glGetProgramiv && api.glUseProgram && api.glGetUniformLocation &&
           api.glUniform1f && api.glUniform2f && api.glUniform1i;
    return g_ok;
}

} // namespace gl3
