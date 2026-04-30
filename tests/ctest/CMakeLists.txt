enable_testing()

include(GoogleTest)

# Get GoogleTest source to use gtest headers and link against gtest libraries.
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(STATUS "GTest not found, fetching it...")

    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        52eb8108c5bdec04579160ae17225d66034bd723
    )

    FetchContent_MakeAvailable(googletest)
endif()

set(TEST_SCRIPT_NAMES
    test_camera_edit_roundtrip
    test_camera_edit_validation
    test_model_edit_normalization
    test_model_edit_roundtrip
    test_model_edit_validation
    test_format_mismatch
)

foreach(script_name IN LISTS TEST_SCRIPT_NAMES)
    add_executable(${script_name} ${script_name}.cpp)

    target_compile_definitions(${script_name} PRIVATE
        TEST_MOTION_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../testdata/motions"
    )

    target_link_libraries(${script_name} PRIVATE
        vmdio
        GTest::gtest_main
    )

    gtest_discover_tests(${script_name})
endforeach()