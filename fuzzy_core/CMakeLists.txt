add_library(fuzzy_core SHARED
    src/fuzzy_value.cpp
    src/operations.cpp
)

target_include_directories(fuzzy_core PUBLIC include)
target_compile_features(fuzzy_core PUBLIC cxx_std_17)
set_target_properties(fuzzy_core PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

add_executable(test_fuzzy_core tests/test_main.cpp)
target_link_libraries(test_fuzzy_core PRIVATE fuzzy_core)

# Копирование DLL для теста можно убрать, если в корне задан
# CMAKE_RUNTIME_OUTPUT_DIRECTORY — тогда .exe и .dll уже в одной папке.