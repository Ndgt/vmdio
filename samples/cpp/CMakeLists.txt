cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(vmdio_cpp_samples LANGUAGES CXX)

# Note:
# The following line is uncommented just for BUILD_CPP_SAMPLES option in the root CMakeLists.txt.
#
# find_package(vmdio REQUIRED PATHS <vmdio_install_path>)

add_executable(CameraMotionImport ${CMAKE_CURRENT_SOURCE_DIR}/src/CameraMotionImport.cpp)
target_link_libraries(CameraMotionImport PRIVATE vmdio)

add_executable(ModelMotionImport ${CMAKE_CURRENT_SOURCE_DIR}/src/ModelMotionImport.cpp)
target_link_libraries(ModelMotionImport PRIVATE vmdio)