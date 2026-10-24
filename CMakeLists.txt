cmake_minimum_required(VERSION 3.15)
project(FuzzyComplex CXX)

# Общая выходная директория, чтобы все .exe и .dll
# оказывались в одной папке и не приходилось копировать вручную
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(fuzzy_core)
add_subdirectory(fuzzy_generator)
add_subdirectory(main)