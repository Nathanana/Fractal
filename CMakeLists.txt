cmake_minimum_required(VERSION 3.16)
project(FractalRenderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/Users/Nathan/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "" FORCE)
endif()

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(Fractal
    src/main.cpp
    src/camera.cpp
    src/shader.cpp
)

target_link_libraries(Fractal PRIVATE glfw glad::glad)

target_include_directories(Fractal PRIVATE include)

add_custom_command(TARGET Fractal POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:Fractal>/shaders
)
