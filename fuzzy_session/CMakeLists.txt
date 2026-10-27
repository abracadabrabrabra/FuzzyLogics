add_library(fuzzy_session SHARED
	src/fuzzy_session.cpp
)
target_include_directories(fuzzy_session PUBLIC include)
target_compile_features(fuzzy_session PUBLIC cxx_std_17)
target_link_libraries(fuzzy_session PUBLIC fuzzy_core)
target_link_libraries(fuzzy_session PUBLIC fuzzy_io)
set_target_properties(fuzzy_session PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
add_executable(tests_fuzzy_session tests/tests.cpp)
target_link_libraries(tests_fuzzy_session PRIVATE fuzzy_session)