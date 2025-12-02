cmake_minimum_required(VERSION 3.10)
project(Fractal)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_library(glad STATIC include/glad/src/glad.c)
target_include_directories(glad PUBLIC include)

set(SOURCES
    src/main.cpp
    src/camera.cpp
    src/shader.cpp
)

add_executable(Fractal ${SOURCES})

if(WIN32)
    set(GLFW_INCLUDE_DIR "path/to/glfw/include")
    set(GLFW_LIBRARY "path/to/glfw/lib/glfw3.lib")
    include_directories(${GLFW_INCLUDE_DIR})
    target_link_libraries(Fractal glad ${GLFW_LIBRARY} opengl32)
elseif(APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)
    include_directories(${GLFW_INCLUDE_DIRS})
    link_directories(${GLFW_LIBRARY_DIRS})
    target_link_libraries(Fractal glad ${GLFW_LIBRARIES} "-framework OpenGL")
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)
    include_directories(${GLFW_INCLUDE_DIRS})
    link_directories(${GLFW_LIBRARY_DIRS})
    target_link_libraries(Fractal glad ${GLFW_LIBRARIES} GL X11 pthread Xrandr Xi dl)
endif()
