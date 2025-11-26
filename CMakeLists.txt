cmake_minimum_required(VERSION 3.16)
project(FractalRenderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake to use vcpkg packages
# (only needed if not provided via command line)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/Users/Nathan/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "" FORCE)
endif()

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(Fractal
    src/main.cpp
    src/App.cpp
    src/Renderer.cpp
    src/Shader.cpp
)

# Link libraries from vcpkg
target_link_libraries(Fractal PRIVATE glfw glad::glad)

# Include dirs (GLM already provided; glad/glfw don't need manual include paths)
target_include_directories(Fractal PRIVATE src)
target_include_directories(Fractal PRIVATE include)

# Copy shaders to the build folder after build
add_custom_command(TARGET Fractal POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:Fractal>/shaders
)
