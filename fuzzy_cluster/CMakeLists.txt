add_library(fuzzy_cluster SHARED
	src/cluster.cpp
)
target_include_directories(fuzzy_cluster PUBLIC include)
target_compile_features(fuzzy_cluster PUBLIC cxx_std_17)
target_link_libraries(fuzzy_cluster PUBLIC fuzzy_core)
set_target_properties(fuzzy_cluster PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
add_executable(test_fuzzy_cluster tests/tests_cluster.cpp)
target_link_libraries(test_fuzzy_cluster PRIVATE fuzzy_cluster fuzzy_generator)