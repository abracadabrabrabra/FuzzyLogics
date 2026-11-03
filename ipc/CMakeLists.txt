add_library(ipc SHARED
    src/ipc.cpp
)

target_include_directories(ipc PUBLIC include)
target_compile_features(ipc PUBLIC cxx_std_17)
set_target_properties(ipc PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

add_executable(tests_ipc tests/test_ipc.cpp)
target_link_libraries(tests_ipc PRIVATE ipc)