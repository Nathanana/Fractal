cmake_minimum_required(VERSION 3.10)
project(Fractal)

set(CMAKE_CXX_STANDARD 17)

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

include_directories(include)

add_library(glad STATIC include/glad/src/glad.c)
target_include_directories(glad PUBLIC include)

set(SOURCES
    src/main.cpp
    src/camera.cpp
    src/shader.cpp
)

add_executable(Fractal ${SOURCES})

file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR}/Release)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(Fractal glad glfw)

if(WIN32)
    target_link_libraries(Fractal opengl32)
elseif(APPLE)
    target_link_libraries(Fractal "-framework OpenGL")
else()
    target_link_libraries(Fractal GL X11 pthread Xrandr Xi dl)
endif()
