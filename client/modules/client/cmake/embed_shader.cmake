# Usage: cmake -DFRAG_IN=post.frag -DFRAG_HDR_OUT=post.frag.h -P embed_shader.cmake
# Produces a header with: namespace crimson_shaders { inline const char* POST_FRAG = R"GLSL(...)GLSL"; }
if(NOT DEFINED FRAG_IN OR NOT DEFINED FRAG_HDR_OUT)
    message(FATAL_ERROR "FRAG_IN and FRAG_HDR_OUT required")
endif()

file(READ ${FRAG_IN} _src)

set(_out "// Auto-generated from post.frag by embed_shader.cmake - do not edit.\n")
string(APPEND _out "#pragma once\n")
string(APPEND _out "namespace crimson_shaders {\n")
string(APPEND _out "inline const char* POST_FRAG = R\"GLSL(")
string(APPEND _out "${_src}")
string(APPEND _out ")GLSL\";\n")
string(APPEND _out "} // namespace crimson_shaders\n")

file(WRITE ${FRAG_HDR_OUT} "${_out}")
message(STATUS "Embedded shader -> ${FRAG_HDR_OUT}")
