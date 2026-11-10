cmake_minimum_required(VERSION 3.20)
#project(MyFuzzyApp)

set(CMAKE_CXX_STANDARD 17)

add_executable(MyFuzzyApp
    src/main.cpp
)

find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(MyFuzzyApp PRIVATE
    fuzzy_core
    fuzzy_generator
    fuzzy_io
    fuzzy_session
    ipc
    imgui::imgui
    implot::implot
    SDL3::SDL3
    OpenGL::GL
)

add_dependencies(MyFuzzyApp fuzzy_cli)