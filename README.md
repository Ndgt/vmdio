[README - 日本語](./README-jp.md)

# vmdio
`vmdio` is a library that provides a simple C++ interface for reading and writing VMD (Vocaloid Motion Data) files used by MMD (MikuMikuDance). This library is also available from Python as a wrapper module called `pyvmdio`.

<br>

## 1. Build Requirements
- C++ Library (`vmdio`)

    - Windows 64-bit (x64) environment with MSVC supporting C++17 or higher
    - CMake 3.25 or higher


- Python Wrapper module and wheel (`pyvmdio`)

    - Build requirements listed above for C++ library
    - [pybind11](https://github.com/pybind/pybind11)
    - Python 3.11 or higher with `build` and `pybind11-stubgen` modules installed

- C++ reference documentation (optional)

    - [Doxygen](https://www.doxygen.nl/)
    - [Graphviz](https://www.graphviz.org/)

- CTest tests (optional)

    - [GoogleTest](https://github.com/google/googletest)

<br>

## 2. Build and Install Library
Get the source code and navigate to the project directory:
```
git clone https://github.com/Ndgt/vmdio.git
cd vmdio
```

<br>The following CMake options are available when configuring the project. See the [CMakeLists.txt](/CMakeLists.txt) for more details.

- `BUILD_CPP_SAMPLES` : Build C++ sample applications (default: `OFF`)

- `BUILD_CTEST` : Build CTest tests (default: `OFF`. `GoogleTest` is automatically fetched)

- `BUILD_PYTHON_WRAPPER` : Build Python Wrapper module and wheel (default: `OFF`)

<br>

### 2.1. Build C++ Library (`vmdio`)
When configuring the project with CMake, set the toolset version to match your Visual Studio version (e.g., `v143` for Visual Studio 17 2022 Generator):

```
cmake -S . -B build -A x64 -T v143
```

<br>Then, choose the build configuration (e.g., `Release`) and run the build command:
```
cmake --build build --config Release
```

<br>You can build and run the CTest for the library if `BUILD_CTEST` is set to `ON`:

```
ctest --test-dir build/tests/ctest -C Release -V
```

<br>After building, you can install the library to a specified path:

```
cmake --install build --prefix <install_path> --config Release --component vmdio
```

<br>

The following directories will be created in `<install_path>`:

- `include` : Header files
- `lib` : Compiled library file and cmake configuration files for `find_package()`


<br>

### 2.2. Build Python Wrapper (`pyvmdio`)
Install build requirements for Python wrapper:

```
pip install build pybind11-stubgen
```
<br>Set the `BUILD_PYTHON_WRAPPER` option to `ON` when configuring with CMake. This will call `FetchContent()` CMake function to download the `pybind11` library if it is not found in the system:

```
cmake -S . -B build -A x64 -T v143 -DBUILD_PYTHON_WRAPPER=ON
```

<br>Run build command. This will also invoke the `pybind11-stubgen` and Python wheel build process:
```
cmake --build build --config Release
```

<br>You can run the pytest for the Python wrapper module:

```
pip install pytest
pip install ./build/wheel/dist/pyvmdio-<version>-<tags>.whl
pytest -s ./tests/pytest
```

<br>

## 3. Documentation
For detailed explanations of the library design and data structures, see the [Technical Architecture](/docs/technical_architecture.md) document.

Including the Technical Architecture document, the C++ reference documentation can be generated using `Doxygen`:

```
doxygen ./docs/Doxyfile
```

<br>Open the generated `./docs/build/html/index.html` to view the documentation.

<br>

## 4. Using the Library

See the following directories for example usage of the `vmdio` library and `pyvmdio` module.

- [C++ samples](/samples/cpp)
- [Python samples](/samples/python)

<br>Also, refer to the [Technical Architecture](/docs/technical_architecture.md) document to understand the design and data structures of the `vmdio` library.

<br>

### 4.1. Note on Handling Strings

Within the public API of this library, strings are handled as `UTF-8` encoded `std::string` values. When using this library, please configure your code editor and terminal to use `UTF-8` encoding.

On the other hand, strings stored in keyframe data fields and similar fields in VMD files are fixed-length `Shift_JIS` fields, and the required encoding conversions are handled internally by the library. When writing, the library throws an exception if a string contains characters that cannot be converted to `Shift_JIS`, or if the encoded byte length exceeds the defined field length. Likewise, when reading, the library throws an exception if decoding from `Shift_JIS` to `UTF-8` fails.

The following setting in [CMakeLists.txt](/CMakeLists.txt) ensures that the library and the project source code are compiled with `UTF-8` encoding.

```cmake
# Set UTF-8 encoding
target_compile_options(vmdio PUBLIC /utf-8)
```

<br>

### 4.2. Validation in Read/Write Functions

The VMD file reading function `readVMD()` is designed to preserve values from the VMD file as faithfully as possible when they can be represented by the library's data structures. However, it throws exceptions when encountering values that cannot be represented by the library's data structures (for example, undefined flag values), or when string decoding fails.

The VMD file writing function `writeVMD()` performs the necessary validation before writing, and throws exceptions when invalid values are detected or when string encoding fails.

`writeVMD()` treats a bone rotation represented as a quaternion as invalid if its norm is close to zero and throws an exception. If the quaternion is non-zero but not normalized, the library normalizes it internally during serialization and writes it. 

<br>

### 4.3. CMake Integration
`vmdio` library is released as a CMake package. In your `CMakeLists.txt`, it is convenient to use `FetchContent()` to fetch this library as a dependency:

```CMake
find_package(vmdio QUIET)

if(NOT vmdio_FOUND)
    message(STATUS "vmdio not found, fetching it...")

    include(FetchContent)

    FetchContent_Declare(
        vmdio
        GIT_REPOSITORY https://github.com/Ndgt/vmdio.git
        GIT_TAG        <commit hash of release version>
    )

    FetchContent_MakeAvailable(vmdio)
endif()
```

<br>Alternatively, if you have installed the library to a specific path, set the path to the `PATHS` option of `find_package()`:

```CMake
find_package(vmdio REQUIRED PATHS <vmdio_install_path>)
```

<br>Then, set the target link libraries to link against `vmdio`:

```CMake
target_link_libraries(${PROJECT_NAME} PRIVATE vmdio)
```

<br>

### 4.4. Python Integration
The Python wrapper module `pyvmdio` is implemented using pybind11, and all identifiers in the `vmdio` library are preserved in the Python wrapper, so the API is almost identical between C++ and Python.

`pyvmdio` module is released as a Python wheel. You can install the wheel using `pip` and then import the module in your Python code.

```
pip install <path to pyvmdio-*.whl>
```

<br>Basic usage in Python code:

```python
import pyvmdio.model_edit as vmdio

vmd_data = vmdio.readVMD("<path to vmd file>")

print(f"Model Name: {vmd_data.modelName}")
```

<br>

## 5. License
This project is licensed under the `BSD 3-Clause License`.

For third-party libraries used in this project, see the [LICENSES](/LICENSES) directory for details.