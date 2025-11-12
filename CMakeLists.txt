cmake_minimum_required(VERSION 4.0)
project(OpenGLProject)

find_package(glfw3 REQUIRED)

set(CMAKE_CXX_STANDARD 20)

add_executable(MyProgram main.cpp glad.c)

target_include_directories(MyProgram PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(MyProgram PRIVATE
glfw
"-framework OpenGL")
