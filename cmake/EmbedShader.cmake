# ------------------------------------------------------------
# Embeds a text file's contents into a generated C++ header
# as a raw string literal.
#
# Usage: cmake -DINPUT_FILE=... -DOUTPUT_FILE=... -DVAR_NAME=... -P EmbedShader.cmake
# ------------------------------------------------------------

file(READ "${INPUT_FILE}" SHADER_SOURCE)

file(WRITE "${OUTPUT_FILE}"
"#pragma once\n"
"// Auto-generated from ${INPUT_FILE} - do not edit directly\n"
"constexpr const char* ${VAR_NAME} = R\"GLSL(\n${SHADER_SOURCE})GLSL\";\n"
)