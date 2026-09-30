#include "client/render/postfx.h"
#include "client/render/gl3w_min.h"
#include "client/render/shaders/post.frag.h"
#include "common/log.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <cstring>
#include <cmath>

namespace postfx {

static const char* TAG = "postfx";
static bool g_init = false;
static bool g_failed = false;
static unsigned g_prog = 0;
static unsigned g_vao = 0, g_vbo = 0;
static int g_loc_time = -1, g_loc_amount = -1, g_loc_res = -1, g_loc_scene = -1;
static double g_last_apply = 0;

// vertex shader: fullscreen triangle strip
static const char* VS_SRC =
    "#version 330\n"
    "const vec2 P[4] = vec2[4](vec2(-1,-1), vec2(1,-1), vec2(-1,1), vec2(1,1));\n"
    "out vec2 v_uv;\n"
    "void main() {\n"
    "    vec2 p = P[gl_VertexID];\n"
    "    v_uv = p * 0.5 + 0.5;\n"
    "    gl_Position = vec4(p, 0.0, 1.0);\n"
    "}\n";

static unsigned compile(unsigned type, const char* src) {
    unsigned s = gl3::api.glCreateShader(type);
    gl3::api.glShaderSource(s, 1, &src, nullptr);
    gl3::api.glCompileShader(s);
    int okflag = 0;
    gl3::api.glGetShaderiv(s, GL_COMPILE_STATUS, &okflag);
    if (!okflag) {
        char logbuf[1024]{};
        gl3::api.glGetShaderInfoLog(s, 1024, nullptr, logbuf);
        clog::error(TAG, "shader compile failed: %s", logbuf);
        gl3::api.glDeleteShader(s);
        return 0;
    }
    return s;
}

void init() {
    if (g_init || g_failed) return;
    if (!gl3::bind()) {
        clog::error(TAG, "GL core binding failed (context too old?) - postfx disabled");
        g_failed = true;
        return;
    }

    unsigned vs = compile(GL_VERTEX_SHADER, VS_SRC);
    unsigned fs = compile(GL_FRAGMENT_SHADER, crimson_shaders::POST_FRAG);
    if (!vs || !fs) { g_failed = true; return; }

    g_prog = gl3::api.glCreateProgram();
    gl3::api.glAttachShader(g_prog, vs);
    gl3::api.glAttachShader(g_prog, fs);
    gl3::api.glLinkProgram(g_prog);
    int linked = 0;
    gl3::api.glGetProgramiv(g_prog, GL_LINK_STATUS, &linked);
    gl3::api.glDeleteShader(vs);
    gl3::api.glDeleteShader(fs);
    if (!linked) {
        char logbuf[1024]{};
        gl3::api.glGetProgramInfoLog(g_prog, 1024, nullptr, logbuf);
        clog::error(TAG, "program link failed: %s", logbuf);
        g_failed = true;
        return;
    }

    gl3::api.glGenVertexArrays(1, &g_vao);
    gl3::api.glBindVertexArray(g_vao);
    gl3::api.glGenBuffers(1, &g_vbo);
    gl3::api.glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    const float quad[] = { // only to satisfy attrib 0; positions from gl_VertexID
        -1.f, -1.f,  1.f, -1.f, -1.f, 1.f,  1.f, 1.f
    };
    gl3::api.glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    gl3::api.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    gl3::api.glEnableVertexAttribArray(0);
    gl3::api.glBindVertexArray(0);
    gl3::api.glBindBuffer(GL_ARRAY_BUFFER, 0);

    g_loc_time = gl3::api.glGetUniformLocation(g_prog, "u_time");
    g_loc_amount = gl3::api.glGetUniformLocation(g_prog, "u_amount");
    g_loc_res = gl3::api.glGetUniformLocation(g_prog, "u_res");
    g_loc_scene = gl3::api.glGetUniformLocation(g_prog, "u_scene");

    g_init = true;
    clog::info(TAG, "postfx ready");
}

void shutdown() {
    if (!g_init) return;
    // Program leak is acceptable: freed at process exit with the context.
    g_init = false;
}

void apply(double t, float amount, int w, int h) {
    if (!g_init) init();
    if (!g_init || g_failed || amount <= 0.001f) return;

    // The scene texture is already bound to unit 0 by the game's renderer at
    // swap time (its final post shader output); we re-draw the frame with our
    // pass sampling whatever is on unit 0.
    gl3::api.glUseProgram(g_prog);
    if (g_loc_time >= 0)   gl3::api.glUniform1f(g_loc_time, (float)t);
    if (g_loc_amount >= 0) gl3::api.glUniform1f(g_loc_amount, amount);
    if (g_loc_res >= 0)    gl3::api.glUniform2f(g_loc_res, (float)w, (float)h);
    if (g_loc_scene >= 0)  gl3::api.glUniform1i(g_loc_scene, 0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    gl3::api.glBindVertexArray(g_vao);
    gl3::api.glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    gl3::api.glBindVertexArray(0);
    gl3::api.glUseProgram(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_last_apply = t;
}

double last_apply_time() { return g_last_apply; }

} // namespace postfx
