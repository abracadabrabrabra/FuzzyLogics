add_library(fuzzy_generator SHARED
	src/poisson_generator.cpp
)
target_include_directories(fuzzy_generator PUBLIC include)
target_compile_features(fuzzy_generator PUBLIC cxx_std_17)
target_link_libraries(fuzzy_generator PUBLIC fuzzy_core)
set_target_properties(fuzzy_generator PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
add_executable(test_fuzzy_generator tests/tests.cpp)
target_link_libraries(test_fuzzy_generator PRIVATE fuzzy_generator)