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

- `BUILD_WITH_STATIC_CRT` : Use `/MT` or `/MTd` as the runtime library option (default: `OFF`. Not supported with `BUILD_PYTHON_WRAPPER`)

<br>

### 2.1. Build C++ Library (`vmdio`)
When configuring the project with CMake, set the toolset version to match your Visual Studio version (e.g., `v143` for Visual Studio 17 2022 Generator):

```
cmake -S . -B build -A x64 -T v143
```

<br>By default, the library is built with `/MD` or `/MDd`. To build with `/MT` or `/MTd`, set the `BUILD_WITH_STATIC_CRT` option to `ON`:

```
cmake -S . -B build -A x64 -T v143 -DBUILD_WITH_STATIC_CRT=ON
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

String fields in the public API are represented by `vmdio::VMDString`.

`VMDString` is a value type for VMD string fields. It stores the `Shift_JIS` byte sequence used by VMD files as its internal representation. This allows the library to preserve string-field bytes read from VMD files as much as possible, including byte sequences that cannot be strictly decoded to `UTF-8`.

For ordinary user-facing text in C++, construct a `VMDString` from a `UTF-8` string:

```cpp
#include <vmdio/model_edit.h>
#include <vmdio/vmd_string.h>

vmdio::model_edit::VMDData lData;
lData.modelName = vmdio::VMDString::fromUTF8(u8"初音ミク");
```

The `model_edit.h` and `camera_edit.h` headers include `vmd_string.h` because their public data structures contain `VMDString` fields. However, when user code directly constructs or converts `VMDString` values, it is recommended to include `vmd_string.h` explicitly for clarity.

If you already have `Shift_JIS` encoded bytes, use `fromShiftJIS()` or `fromShiftJISBytes()`:

```cpp
vmdio::VMDString lName = vmdio::VMDString::fromShiftJIS(lShiftJISBytes);
```

To obtain text for display, use `toUTF8ForDisplay()`. Invalid or incomplete byte sequences are replaced with `?` in this display-oriented conversion.

```cpp
std::string lDisplayName = lName.toUTF8ForDisplay();
```

For strict conversion, use `toUTF8()`. This may throw an exception if the stored bytes cannot be decoded as `Shift_JIS`.

```cpp
std::string lStrictUTF8Name = lName.toUTF8();
```

<br>When writing VMD files, the library validates the stored `Shift_JIS` byte length at the serialization boundary and throws an exception if the field exceeds the VMD field-size limit. It does not silently truncate strings.

<br>In the Python wrapper, VMD string fields accept `str`, `bytes`, or `VMDString` values.

```python
import pyvmdio.model_edit as vmdio

data = vmdio.VMDData()
data.modelName = "初音ミク"          # Python str

frame = vmdio.MotionFrame()
frame.boneName = "センター"          # converted to VMDString internally
```

When a Python `str` is assigned to a VMD string field, it is converted to `VMDString` immediately. If the text cannot be represented in `Shift_JIS`, `pyvmdio.exceptions.StringProcessError` is raised at assignment time. Field byte-length limits are still validated when `writeVMD()` serializes the data.

Python `bytes` assigned to VMD string fields are treated as UTF-8 encoded bytes. If you already have raw `Shift_JIS` bytes, explicitly construct a `VMDString` with `VMDString.fromShiftJIS()`.

```python
from pyvmdio.vmd_string import VMDString

frame.boneName = "センター".encode("utf-8")
frame.boneName = VMDString.fromShiftJIS("センター".encode("cp932"))
```

The Python encoding API represents encoded byte sequences as `bytes`:

```python
import pyvmdio.encoding as encoding

shift_jis_bytes = encoding.utf8ToShiftJIS("センター")                  # str -> bytes
shift_jis_bytes = encoding.utf8ToShiftJIS("センター".encode("utf-8"))  # UTF-8 bytes -> bytes
text = encoding.shiftJISToUTF8(shift_jis_bytes)                       # bytes -> str
```

<br>The following setting in [CMakeLists.txt](/CMakeLists.txt) ensures that the library and the project source code are compiled with `UTF-8` encoding.

```cmake
# Set UTF-8 encoding
target_compile_options(vmdio PUBLIC /utf-8)
```

<br>

### 4.2. Validation in Read/Write Functions

The VMD file reading function `readVMD()` is designed to preserve values from the VMD file as faithfully as possible when they can be represented by the library's data structures. It throws exceptions when encountering values that cannot be represented by those structures, such as undefined flag values.

For VMD string fields, `readVMD()` preserves the meaningful `Shift_JIS` bytes before the first NUL byte. Incomplete or invalid `Shift_JIS` byte sequences are kept in `VMDString` and can be displayed with `toUTF8ForDisplay()`, which replaces invalid parts with `?`.

The VMD file writing function `writeVMD()` performs validation before writing. It throws exceptions when invalid values are detected or when a stored `Shift_JIS` string exceeds the VMD field-size limit.

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

The Python wrapper module `pyvmdio` is implemented using pybind11. The wrapper follows the C++ API structure as closely as possible, while mapping encoded byte sequences to Python's `bytes` type where appropriate.

`pyvmdio` module is released as a Python wheel. You can install the wheel using `pip` and then import the module in your Python code.

```
pip install <path to pyvmdio-*.whl>
```

<br>Basic usage in Python code:

```python
import pyvmdio.model_edit as vmdio

vmd_data = vmdio.readVMD("<path to vmd file>")

print(f"Model Name: {vmd_data.modelName.toUTF8ForDisplay()}")
```

<br>For ordinary text, VMD string fields can be assigned with Python `str` values:

```python
data = vmdio.VMDData()
data.modelName = "初音ミク"

frame = vmdio.MotionFrame()
frame.boneName = "センター"
```

<br>Python `bytes` assigned to VMD string fields are treated as UTF-8 encoded bytes. To use raw `Shift_JIS` bytes, construct a `VMDString` explicitly.

```python
from pyvmdio.vmd_string import VMDString

frame.boneName = "センター".encode("utf-8")
frame.boneName = VMDString.fromShiftJIS("センター".encode("cp932"))

print(frame.boneName.toShiftJIS())
```

<br>The Python encoding functions use `str` for Unicode text and `bytes` for encoded byte sequences:

```python
import pyvmdio.encoding as encoding

shift_jis_bytes = encoding.utf8ToShiftJIS("センター")
text = encoding.shiftJISToUTF8(shift_jis_bytes)
```

<br>**Note:** Python frame list classes are backed by C++ `std::vector`. Reacquire elements after mutating a frame list.

<br>

## 5. License
This project is licensed under the `BSD 3-Clause License`.

For third-party libraries used in this project, see the [LICENSES](/LICENSES) directory for details.