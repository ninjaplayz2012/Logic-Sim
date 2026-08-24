cmake_minimum_required(VERSION 3.20)
project(LogicSim LANGUAGES CXX)

# ------------------------------------------------------------
# C++ Standard
# ------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# Source Files
# ------------------------------------------------------------

file(GLOB_RECURSE SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# ------------------------------------------------------------
# Executable
# ------------------------------------------------------------

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

# ------------------------------------------------------------
# Include Directories
# ------------------------------------------------------------

target_include_directories(${PROJECT_NAME} PRIVATE

    # Your project headers
    "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"

    # GLFW
    "C:/GLFW"

    # GLM
    "C:/glm"

    # GLAD
    "C:/glad"

    # stb_image
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
endif()


# ------------------------------------------------------------
# GLAD
# ------------------------------------------------------------

target_sources(${PROJECT_NAME} PRIVATE
    "C:/glad/glad.c"
)