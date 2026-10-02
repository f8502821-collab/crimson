# ---------------------------------------------------------------------------
# External dependencies: ImGui, MinHook, fonts, shader embedding
# All fetched at configure time. Internet is required on FIRST configure only.
# ---------------------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

# --- Dear ImGui (D3D11 backend for injector, OpenGL3 backend for client) ----
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8
    GIT_SHALLOW TRUE
)

# --- MinHook (x64 inline hooks) ---------------------------------------------
FetchContent_Declare(minhook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG v1.3.3
    GIT_SHALLOW TRUE
)

# MinHook v1.3.3 is the final upstream release; do not bump.
# (This project is 1.21.11-only and never updated.)

FetchContent_MakeAvailable(imgui minhook)

# --- Fonts (Gruppo for display, JetBrainsMono for numbers) ------------------
# Graceful fallback: if the download fails we keep going; code falls back to
# the built-in ProggyClean/Segoe at runtime when the font files are absent.
set(CRIMSON_FONT_DIR ${CMAKE_BINARY_DIR}/fonts)
file(MAKE_DIRECTORY ${CRIMSON_FONT_DIR})

function(crimson_download_font url out_path)
    if(NOT EXISTS ${out_path})
        file(DOWNLOAD ${url} ${out_path}
            TIMEOUT 60
            STATUS dl_status
        )
        list(GET dl_status 0 dl_code)
        if(NOT dl_code EQUAL 0)
            message(WARNING "Font download failed (${url}); runtime will fall back to default font.")
            file(REMOVE ${out_path})
        endif()
    endif()
endfunction()

crimson_download_font(
    https://github.com/google/fonts/raw/main/ofl/gruppo/Gruppo-Regular.ttf
    ${CRIMSON_FONT_DIR}/Gruppo-Regular.ttf
)
crimson_download_font(
    https://github.com/JetBrains/JetBrainsMono/raw/master/fonts/ttf/JetBrainsMono-Regular.ttf
    ${CRIMSON_FONT_DIR}/JetBrainsMono-Regular.ttf
)

# --- Shader embedding: post.frag -> post.frag.h -----------------------------
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

add_custom_command(
    OUTPUT ${GENERATED_DIR}/post.frag.h
    COMMAND ${CMAKE_COMMAND}
        -DFRAG_IN=${CMAKE_CURRENT_SOURCE_DIR}/client/render/shaders/post.frag
        -DFRAG_HDR_OUT=${GENERATED_DIR}/post.frag.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_shader.cmake
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/client/render/shaders/post.frag
    COMMENT "Embedding GLSL post shader"
    VERBATIM
)
add_custom_target(crimson_embed_shader DEPENDS ${GENERATED_DIR}/post.frag.h)
# NOTE: crimson_client adds itself as dependent in the root CMakeLists (this
# file runs before that target exists).
