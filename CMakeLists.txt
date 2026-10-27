cmake_minimum_required(VERSION 3.15)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" _vcpkg_root_cmake)
    set(CMAKE_TOOLCHAIN_FILE "${_vcpkg_root_cmake}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()
project(FuzzyComplex CXX)

# Общая выходная директория, чтобы все .exe и .dll
# оказывались в одной папке и не приходилось копировать вручную
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(fuzzy_core)
add_subdirectory(fuzzy_generator)
add_subdirectory(fuzzy_io)
add_subdirectory(fuzzy_session)
add_subdirectory(fuzzy_cli)
add_subdirectory(main)