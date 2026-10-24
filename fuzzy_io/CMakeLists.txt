add_library(fuzzy_io SHARED
	src/fuzzy_io.cpp
)
target_include_directories(fuzzy_io PUBLIC include)
target_compile_features(fuzzy_io PUBLIC cxx_std_17)
target_link_libraries(fuzzy_io PUBLIC fuzzy_core)
set_target_properties(fuzzy_io PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
add_executable(sample_fuzzy_io sample/sample.cpp)
target_link_libraries(sample_fuzzy_io PRIVATE fuzzy_io)