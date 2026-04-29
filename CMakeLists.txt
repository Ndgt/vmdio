cmake_minimum_required(VERSION 3.25)

if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>")
endif()

file(STRINGS "VERSION.txt" SPECIFIED_VERSION LIMIT_COUNT 1)

project(vmdio VERSION ${SPECIFIED_VERSION} LANGUAGES CXX)

# Check the architecture, only MSVC 64-bit is supported for now.
if(NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Only 64-bit MSVC builds are supported.")
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# ------------------------------------
# Configurations
# ------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_DEBUG_POSTFIX d)
set(CACHE{FETCHCONTENT_BASE_DIR} TYPE PATH FORCE VALUE "${CMAKE_CURRENT_SOURCE_DIR}/_deps")

# ------------------------------------
# command line build options
# ------------------------------------
option(BUILD_CTEST "Build CTest tests" OFF)
option(BUILD_CPP_SAMPLES "Build C++ sample applications" OFF)
option(BUILD_PYTHON_WRAPPER "Build Python Wrapper module and wheel" OFF)

# ------------------------------------
# vmdio library
# ------------------------------------
add_library(vmdio STATIC
    src/vmdio/camera_edit.cpp
    src/vmdio/model_edit.cpp
    src/vmdio/vmd_encoding.cpp
    src/vmdio/vmd_string.cpp
)

# Set UTF-8 encoding
target_compile_options(vmdio PUBLIC /utf-8)

target_sources(vmdio PUBLIC
    FILE_SET HEADERS
    BASE_DIRS src
    FILES
        src/vmdio/camera_edit.h
        src/vmdio/model_edit.h
        src/vmdio/vmd_encoding.h
        src/vmdio/vmd_exceptions.h
        src/vmdio/vmd_string.h
)

set(INSTALL_CONFIG_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/vmdio")
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/vmdio")

configure_package_config_file(
    src/cmake/vmdioConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/vmdioConfig.cmake"
    INSTALL_DESTINATION "${INSTALL_CONFIG_DIR}"
    PATH_VARS INCLUDE_INSTALL_DIR
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/vmdioConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(TARGETS vmdio
    EXPORT vmdioTargets
    ARCHIVE COMPONENT vmdio
    FILE_SET HEADERS COMPONENT vmdio
)

install(EXPORT vmdioTargets
    FILE vmdioTargets.cmake
    DESTINATION "${INSTALL_CONFIG_DIR}"
    COMPONENT vmdio
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/vmdioConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/vmdioConfigVersion.cmake"
    DESTINATION "${INSTALL_CONFIG_DIR}"
    COMPONENT vmdio
)

# -------------------------------------
# C++ samples
# -------------------------------------
if(BUILD_CPP_SAMPLES)
    add_subdirectory(samples/cpp)
endif()

# -------------------------------------
# CTest
# -------------------------------------
if(BUILD_CTEST)
    add_subdirectory(tests/ctest)
endif()

# -------------------------------------
# Python wrapper
# -------------------------------------
if(BUILD_PYTHON_WRAPPER)
    set(PYTHON_WRAPPER_NAME pyvmdio)
    set(PYTHON_WRAPPER_LIBRARY_NAME _${PYTHON_WRAPPER_NAME})

    # Set the output directory and create it
    set(PYTHON_WRAPPER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${PYTHON_WRAPPER_NAME}")
    set(PYTHON_WHEEL_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/wheel")
    file(MAKE_DIRECTORY "${PYTHON_WRAPPER_OUTPUT_DIR}")
    file(MAKE_DIRECTORY "${PYTHON_WHEEL_BUILD_DIR}")

    # Find Python 3.11 or higher
    find_package(Python 3.11 REQUIRED COMPONENTS Interpreter Development)

    # Find pybind11, if not found, fetch it using FetchContent
    find_package(pybind11 CONFIG QUIET)

    if(NOT pybind11_FOUND)
        message(STATUS "pybind11 not found, fetching it...")

        include(FetchContent)

        FetchContent_Declare(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git
            GIT_TAG        d03662f0984f652b60e7ddce53d3868002275197 # Release v3.0.4
        )

        FetchContent_MakeAvailable(pybind11)
    endif()

    # Python wrapper configuration
    pybind11_add_module(${PYTHON_WRAPPER_LIBRARY_NAME} src/pyvmdio/pyvmdio.cpp)

    set_target_properties(${PYTHON_WRAPPER_LIBRARY_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PYTHON_WRAPPER_OUTPUT_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PYTHON_WRAPPER_OUTPUT_DIR}"
    )

    configure_file(src/pyvmdio/config/__init__.py.in "${PYTHON_WRAPPER_OUTPUT_DIR}/__init__.py")

    target_include_directories(${PYTHON_WRAPPER_LIBRARY_NAME} PRIVATE src)

    target_link_libraries(${PYTHON_WRAPPER_LIBRARY_NAME} PRIVATE vmdio)

    # Python module configuration
    # Collect all necessary files for the Python module to the output directory,
    # and run pybind11-stubgen to generate stub files for the module. 
    add_custom_command(
        TARGET ${PYTHON_WRAPPER_LIBRARY_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src/pyvmdio/submodules" "${PYTHON_WRAPPER_OUTPUT_DIR}"
        COMMAND ${Python_EXECUTABLE} -m pybind11_stubgen -o . ${PYTHON_WRAPPER_NAME}
        WORKING_DIRECTORY "${PYTHON_WRAPPER_OUTPUT_DIR}/.."
    )

    # Configure and build Python wheel
    # Collect Python module sources and documents to the wheel build directory, 
    # and run the Python wheel build command. 
    add_custom_command(
        TARGET ${PYTHON_WRAPPER_LIBRARY_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src/pyvmdio/wheel" "${PYTHON_WHEEL_BUILD_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${PYTHON_WRAPPER_OUTPUT_DIR}" "${PYTHON_WHEEL_BUILD_DIR}/${PYTHON_WRAPPER_NAME}"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/README.md" "${PYTHON_WHEEL_BUILD_DIR}/README.md"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt" "${PYTHON_WHEEL_BUILD_DIR}/LICENSE.txt"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/LICENSES/pybind11/LICENSE" "${PYTHON_WHEEL_BUILD_DIR}/pybind11/LICENSE"
        COMMAND ${Python_EXECUTABLE} -m build --wheel -o dist .
        WORKING_DIRECTORY "${PYTHON_WHEEL_BUILD_DIR}"
    )
endif()