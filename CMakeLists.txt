
cmake_minimum_required(VERSION 3.20)
project(LogicSim LANGUAGES C CXX ASM_NASM)

# ------------------------------------------------------------
# C++ Standard
# ------------------------------------------------------------

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# NASM x64 Assembly
# ------------------------------------------------------------

set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)

# ------------------------------------------------------------
# Source Files
# ------------------------------------------------------------

file(GLOB_RECURSE CPP_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE ASM_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.asm"
)

# ------------------------------------------------------------
# Executable
# ------------------------------------------------------------

add_executable(${PROJECT_NAME}
    ${CPP_SOURCES}
    ${ASM_SOURCES}
)

# ------------------------------------------------------------
# Include Directories
# ------------------------------------------------------------

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "C:/GLFW"
    "C:/glm"
    "C:/glad"
    "C:/stb"
)

# ------------------------------------------------------------
# GLFW
# ------------------------------------------------------------

find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw3
)

# ------------------------------------------------------------
# Windows Libraries
# ------------------------------------------------------------

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        user32
        gdi32
        shell32
    )

    target_link_options(${PROJECT_NAME} PRIVATE -municode)
endif()

# ------------------------------------------------------------
# GLAD
# ------------------------------------------------------------

target_sources(${PROJECT_NAME} PRIVATE
    "C:/glad/glad.c"
)

# ------------------------------------------------------------
# Embed Shaders
# ------------------------------------------------------------

file(GLOB_RECURSE SHADER_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/*.frag"
)

set(GENERATED_SHADER_DIR "${CMAKE_BINARY_DIR}/GeneratedShaders")
file(MAKE_DIRECTORY "${GENERATED_SHADER_DIR}")

set(GENERATED_SHADER_HEADERS "")

foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME "${SHADER_FILE}" NAME)
    string(MAKE_C_IDENTIFIER "${SHADER_NAME}" VAR_NAME)
    set(OUTPUT_HEADER "${GENERATED_SHADER_DIR}/${SHADER_NAME}.hpp")

    add_custom_command(
        OUTPUT "${OUTPUT_HEADER}"
        COMMAND ${CMAKE_COMMAND}
            "-DINPUT_FILE=${SHADER_FILE}"
            "-DOUTPUT_FILE=${OUTPUT_HEADER}"
            "-DVAR_NAME=${VAR_NAME}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShader.cmake"
        DEPENDS
            "${SHADER_FILE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShader.cmake"
        COMMENT "Embedding shader ${SHADER_NAME}"
        VERBATIM
    )

    list(APPEND GENERATED_SHADER_HEADERS "${OUTPUT_HEADER}")
endforeach()

add_custom_target(EmbedShaders
    DEPENDS ${GENERATED_SHADER_HEADERS}
)

add_dependencies(${PROJECT_NAME} EmbedShaders)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${GENERATED_SHADER_DIR}"
)

# ------------------------------------------------------------
# CPU Instructions and Optimization
# ------------------------------------------------------------

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX2)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<COMPILE_LANGUAGE:C,CXX>:-march=native>
        $<$<COMPILE_LANGUAGE:C,CXX>:-O3>
    )
endif()
