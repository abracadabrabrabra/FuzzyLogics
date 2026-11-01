cmake_minimum_required(VERSION 3.20)
project(MyFuzzyApp)

set(CMAKE_CXX_STANDARD 17)

add_executable(MyFuzzyApp
    src/main.cpp
)

find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(MyFuzzyApp PRIVATE
    imgui_implot
    SDL3::SDL3
    OpenGL::GL
)