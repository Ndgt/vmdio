## Technical Architecture

### 1. Overview
VMD (Vocaloid Motion Data) files are divided into two types of data, which are strictly distinguished by MMD during import. A VMD file for model edit data is rejected when imported in camera edit mode, and a VMD file for camera edit data is rejected when imported in model edit mode.

The library is designed to reflect this strict format separation in its public API. Model edit VMD files and camera edit VMD files are handled by separate namespaces and separate public headers.

| Header file | Main API | Use when |
| --- | --- | --- |
| `model_edit.h` | `vmdio::model_edit` | Reading and writing model edit VMD files. |
| `camera_edit.h` | `vmdio::camera_edit` | Reading and writing camera edit VMD files. |
| `vmd_string.h` | `vmdio::VMDString` | Directly constructing, converting, or inspecting VMD string field values. |
| `vmd_encoding.h` | `vmdio::encoding` | Directly using `UTF-8` / Shift_JIS conversion functions. |
| `vmd_exceptions.h` | `vmdio::exceptions` | Catching library-specific exception classes. |

<br>You can usually include only the header for the VMD file type you want to handle:

```cpp
#include <vmdio/model_edit.h>
```

or:

```cpp
#include <vmdio/camera_edit.h>
```

If your code directly constructs or converts `vmdio::VMDString` values, include `vmd_string.h` explicitly for clarity:

```cpp
#include <vmdio/model_edit.h>
#include <vmdio/vmd_string.h>
```

<br>Basic usage in C++ looks like this:

```cpp
#include <vmdio/model_edit.h>
#include <vmdio/vmd_exceptions.h>

#include <exception>
#include <iostream>

namespace vmd = vmdio::model_edit;
namespace vmd_except = vmdio::exceptions;

int main()
{
    try
    {
        vmd::VMDData lVmdData = vmd::readVMD("path_to_vmd_file.vmd");

        /* Some processing of key frame data in the VMDData structure... */

        vmd::writeVMD(lVmdData, "path_to_output_vmd_file.vmd");
    }
    catch (const vmd_except::VMDIOError &e)
    {
        std::cerr << "vmdio library threw an error: " << e.what() << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}
```

<br>

### 2. Read/Write Functions
The main functions for reading and writing VMD files are `readVMD()` and `writeVMD()`, which are defined in both `vmdio::camera_edit` and `vmdio::model_edit` namespaces.

`readVMD()` interprets binary data from a VMD file and populates a `VMDData` structure with more user-friendly data structures.

```cpp
VMDData readVMD(const std::filesystem::path &pFilePath);
```

`writeVMD()` takes a `VMDData` structure and serializes it back into the binary VMD file format.

```cpp
void writeVMD(const VMDData &pVmdData, const std::filesystem::path &pFilePath);
```

During reading and writing, custom exceptions defined in `vmdio::exceptions` may be thrown for errors such as file I/O failures, incompatible formats, invalid field values, string field serialization failures, or total frame counts exceeding the supported limit.

`readVMD()` preserves values from the VMD file as faithfully as possible when they can be represented by the library's data structures. It throws exceptions for values that cannot be represented by those structures, such as undefined enum-backed flag values.

For string fields, `readVMD()` preserves the meaningful `Shift_JIS` bytes before the first NUL byte in each fixed-length VMD string field. Incomplete or invalid `Shift_JIS` byte sequences are kept in `vmdio::VMDString` and can be displayed with `toUTF8ForDisplay()`.

`writeVMD()` validates data before serialization and throws exceptions when invalid values are detected. String field byte-length limits are validated at this serialization boundary. The current validation rules are as follows:

- `vmdio::model_edit`
  - total number of frames exceeds the library limit
  - empty model name
  - duplicate `MotionFrame` entries with the same `boneName` and same `frameNumber`
  - duplicate `MorphFrame` entries with the same `morphName` and same `frameNumber`
  - duplicate `VisibleIKFrame` entries with the same `frameNumber`
  - empty name fields in `MotionFrame` / `MorphFrame` / `IKData`
  - invalid values of `Visibility` / `IKState`
  - norm of `MotionFrame::rotation` is close to zero
  - out-of-range values in `ControlPointSet`
  - string fields whose stored Shift_JIS byte length exceeds the VMD field size

- `vmdio::camera_edit`
  - total number of frames exceeds the library limit
  - duplicate `CameraFrame` / `LightFrame` / `SelfShadowFrame` entries with the same `frameNumber`
  - invalid values of `ProjectionType` / `SelfShadowMode`
  - out-of-range values in `ControlPointSet`

<br>If `MotionFrame::rotation` is non-zero but not unit length, it is normalized internally during serialization. `MorphFrame::value`, `CameraFrame::viewingAngle`, `LightFrame::color`, and `SelfShadowFrame::shadowRange` are not validated against the usual MMD UI slider ranges, since MMD can still read and write values outside those ranges when entered directly.

<br>

### 3. VMD Data Structure
The `VMDData` structure serves as the main container for all the data read from or to be written to a VMD file, holding the vector of key frame data structures for each type of frame.

For model edit VMD files, the structure is defined as follows:

```cpp
// vmdio::model_edit::VMDData

struct VMDData
{
    vmdio::VMDString modelName;
    std::vector<MotionFrame> motionFrames;
    std::vector<MorphFrame> morphFrames;
    std::vector<VisibleIKFrame> visibleIKFrames;
};
```

For camera edit VMD files, the structure is defined as follows:

```cpp
// vmdio::camera_edit::VMDData

struct VMDData
{
    std::vector<CameraFrame> cameraFrames;
    std::vector<LightFrame> lightFrames;
    std::vector<SelfShadowFrame> selfShadowFrames;
};
```

<br>The `VMDData` structure does not represent all raw fields in the VMD file format. The file header is handled internally by the library. The model name field in camera edit VMD files is also handled internally, since MMD uses a fixed model-name marker to distinguish camera edit VMD files from model edit VMD files.

<br>

### 4. Key Frame Data Structures

The following key frame data structures are defined in the library:

| Namespace | Defined frame data structures |
| --- | --- |
| `vmdio::camera_edit` | `CameraFrame`, `LightFrame`, `SelfShadowFrame` |
| `vmdio::model_edit` | `MotionFrame`, `MorphFrame`, `VisibleIKFrame` |

<br>Each frame data structure contains fields that represent the keyframe data for that particular type of frame.

Example model edit frame:

```cpp
// vmdio::model_edit::MotionFrame

struct MotionFrame
{
    vmdio::VMDString boneName;
    uint32_t frameNumber = 0;
    Position position;
    Quaternion rotation;
    MotionInterpolation interpolation;
};
```

Example camera edit frame:

```cpp
// vmdio::camera_edit::CameraFrame

struct CameraFrame
{
    uint32_t frameNumber = 0;
    float distance = 45.00f;
    Position position;
    CameraRotation rotation;
    CameraInterpolation interpolation;
    int32_t viewingAngle = 30;
    ProjectionType projectionType = ProjectionType::Perspective;
};
```

<br>All frame data structures are initialized with default values for their fields. String fields are initialized to empty `vmdio::VMDString` values. Other default values are set to match MMD's usual UI defaults where applicable.

<br>

### 5. Value Data Types and Structures
The following value types and structures are defined in the library:

| Namespace / scope | Defined value types and structures |
| --- | --- |
| `vmdio::camera_edit` | `ProjectionType`, `SelfShadowMode`, `camera_edit::Position`, `CameraRotation`, `camera_edit::ControlPointSet`, `CameraInterpolation`, `Color` |
| `vmdio::model_edit` | `IKState`, `Visibility`, `model_edit::Position`, `Quaternion`, `model_edit::ControlPointSet`, `MotionInterpolation` |
| `vmdio` | `VMDString` |

<br>These are designed to be more user-friendly compared to the raw byte-level representation in the VMD file format.

For example, flag fields are represented by enum classes instead of raw bytes:

```cpp
enum class IKState : uint8_t
{
    OFF = 0,
    ON = 1
};
```

Numerical vector-like values are represented by semantic structures instead of raw float arrays:

```cpp
struct Position
{
    float x;
    float y;
    float z;
};
```

Interpolation data is represented by control point structures instead of raw interpolation byte arrays:

```cpp
struct ControlPointSet
{
    uint32_t x1 = 20;
    uint32_t y1 = 20;
    uint32_t x2 = 107;
    uint32_t y2 = 107;
};

struct MotionInterpolation
{
    ControlPointSet xPos;
    ControlPointSet yPos;
    ControlPointSet zPos;
    ControlPointSet rot;
};
```

<br>Although `Position` and `ControlPointSet` have identical definitions in both `vmdio::model_edit` and `vmdio::camera_edit`, they are intentionally defined separately. This preserves the architectural separation between model edit data and camera edit data, and allows users to include only the header for the VMD file type they want to handle.

String fields are represented by `vmdio::VMDString`. See the next section for details.

<br>

### 6. VMDString

`vmdio::VMDString` is the public value type used for VMD string fields. It is defined in `vmd_string.h`.

VMD files store string fields as fixed-length Shift_JIS byte fields. `VMDString` represents the meaningful string bytes of those fields and keeps Shift_JIS bytes as its internal representation.

This design exists to avoid silent truncation in the middle data model. For example, PMX model data may contain names that are longer than the corresponding VMD string field limit. The library should not silently truncate those names when they are assigned to a VMD data structure. Instead, the value is kept as a `VMDString`, and the field-size limit is validated when `writeVMD()` serializes the data.

When reading, fixed-length VMD string fields are read as Shift_JIS byte sequences. Padding bytes after the first NUL byte are not kept in the middle data model, but the actual string bytes before the NUL byte are preserved as much as possible. This includes incomplete or invalid Shift_JIS byte sequences caused by other tools truncating a string field in the middle of a multi-byte character.

When writing, the library validates the stored Shift_JIS byte length at the serialization boundary and throws an exception if the byte length exceeds the field size limit. The library does not silently truncate string fields.

The field limits are as follows:

| Field | Byte length limit |
| --- | --- |
| `vmdio::model_edit::VMDData::modelName` | 20 bytes |
| `vmdio::model_edit::MotionFrame::boneName` | 15 bytes |
| `vmdio::model_edit::MorphFrame::morphName` | 15 bytes |
| `vmdio::model_edit::IKData::ikBoneName` | 20 bytes |

<br>For ordinary user-facing text, construct a `VMDString` from `UTF-8`:

```cpp
#include <vmdio/model_edit.h>
#include <vmdio/vmd_string.h>

vmdio::model_edit::MotionFrame lFrame;
lFrame.boneName = vmdio::VMDString::fromUTF8(u8"センター");
```

<br>If `Shift_JIS` encoded bytes are already available, construct it from `Shift_JIS`:

```cpp
std::string lShiftJISBytes = /* Shift_JIS encoded bytes */;
vmdio::VMDString lName = vmdio::VMDString::fromShiftJIS(lShiftJISBytes);
```

<br>If the bytes are represented as `std::byte`, use `fromShiftJISBytes()`:

```cpp
const std::byte *lBytes = /* Shift_JIS bytes */;
std::size_t lSize = /* byte count */;

vmdio::VMDString lName = vmdio::VMDString::fromShiftJISBytes(lBytes, lSize);
```

<br>For display-oriented conversion, use `toUTF8ForDisplay()`. Invalid or incomplete `Shift_JIS` byte sequences are replaced with `?`.

```cpp
std::string lDisplayName = lName.toUTF8ForDisplay();
```

<br>For strict conversion, use `toUTF8()`. This may throw an exception if the stored bytes cannot be decoded as `Shift_JIS`.

```cpp
std::string lUTF8Name = lName.toUTF8();
```

<br>To obtain the stored `Shift_JIS` byte sequence as a `std::string`, use `toShiftJIS()`:

```cpp
std::string lShiftJISName = lName.toShiftJIS();
```

<br>The main construction functions are:

```cpp
static VMDString fromUTF8(std::string_view pUTF8String);

static VMDString fromShiftJIS(std::string_view pShiftJISBytes);

static VMDString fromShiftJISBytes(const std::byte *pBytes, std::size_t pSize);
```

<br>The main accessors are:

```cpp
std::string toShiftJIS() const;

std::string toUTF8() const;

std::string toUTF8ForDisplay(bool pStopAtNul = true) const;

std::size_t sizeofShiftJISBytes() const noexcept;

const std::vector<std::byte> &shiftJISBytes() const noexcept;
```

<br>Headers such as `model_edit.h` include `vmd_string.h` because their public structures expose `VMDString` fields. However, user code should include `vmd_string.h` explicitly when it directly constructs or converts `VMDString` values. This makes the dependency visible in user code and avoids relying on transitive includes.

<br>

#### Python wrapper behavior

In the Python wrapper, `VMDString` is exposed as `pyvmdio.VMDString` and `pyvmdio.vmd_string.VMDString`.

Python VMD string fields accept `str`, `bytes`, or `VMDString` values:

```python
import pyvmdio.model_edit as vmdio
from pyvmdio.vmd_string import VMDString

frame = vmdio.MotionFrame()

frame.boneName = "センター"                                      # Python str
frame.boneName = "センター".encode("utf-8")                     # UTF-8 bytes
frame.boneName = VMDString.fromUTF8("センター")                 # explicit VMDString
frame.boneName = VMDString.fromShiftJIS("センター".encode("cp932"))  # raw Shift_JIS bytes
```

<br>`str` and UTF-8 `bytes` are converted to `VMDString` at assignment time. If the text cannot be represented in `Shift_JIS`, `pyvmdio.exceptions.StringProcessError` is raised.

To use raw `Shift_JIS` bytes, do not assign `bytes` directly to a string field. Explicitly construct a `VMDString` with `VMDString.fromShiftJIS()`.

Field byte-length limits are not checked at assignment time. They are checked when `writeVMD()` serializes the data, matching the C++ API design.

`VMDString.toShiftJIS()` returns Python `bytes`, while `VMDString.toUTF8()` and `VMDString.toUTF8ForDisplay()` return Python `str`.

```python
name = VMDString.fromUTF8("センター")

shift_jis_bytes = name.toShiftJIS()
display_text = name.toUTF8ForDisplay()
```

<br>Python frame list classes are backed by C++ `std::vector`. Do not keep references to elements obtained from a list, such as `frame = data.motionFrames[0]`, while mutating the same list with `append`, `insert`, `extend`, `remove`, `clear`, or similar operations. After mutating a list, retrieve the element again from the list.

<br>

### 7. Encoding Conversion

Including the `vmd_encoding.h` header allows you to use the encoding conversion functions defined in the `vmdio::encoding` namespace:

```cpp
std::string utf8ToShiftJIS(std::string_view pString);

std::string shiftJISToUTF8(std::string_view pString);
```

These functions are used internally by the library for conversions between `UTF-8` and `Shift_JIS`. They can also be used directly by library users when needed.

In the C++ API, both UTF-8 text and Shift_JIS encoded byte sequences are represented by `std::string`. Therefore, the semantic meaning of the string depends on the function:

- `utf8ToShiftJIS()` accepts UTF-8 text and returns Shift_JIS encoded bytes as `std::string`.
- `shiftJISToUTF8()` accepts Shift_JIS encoded bytes as `std::string` and returns UTF-8 text.

`VMDString::fromUTF8()` uses UTF-8 to Shift_JIS conversion internally. `VMDString::toUTF8()` and `VMDString::toUTF8ForDisplay()` use Shift_JIS to UTF-8 conversion internally, with `toUTF8ForDisplay()` replacing invalid or incomplete byte sequences with `?`.

In the Python wrapper, this distinction is represented with Python's `str` and `bytes` types:

```python
import pyvmdio.encoding as encoding

shift_jis_bytes = encoding.utf8ToShiftJIS("センター")                  # str -> bytes
shift_jis_bytes = encoding.utf8ToShiftJIS("センター".encode("utf-8"))  # UTF-8 bytes -> bytes
text = encoding.shiftJISToUTF8(shift_jis_bytes)                       # bytes -> str
```

In Python, `encoding.utf8ToShiftJIS()` accepts either `str` or UTF-8 encoded `bytes` and returns `Shift_JIS` `bytes`. `encoding.shiftJISToUTF8()` accepts `Shift_JIS` `bytes` and returns `str`.

<br>

### 8. Custom Exceptions

The `vmd_exceptions.h` header defines the following custom exception classes for error handling in the library:

| Exception class | Brief description |
| --- | --- |
| `VMDIOError` | Base class for all exceptions in the library. |
| `FileSystemError` | Exception related to file I/O operations. |
| `FrameConflictError` | Exception thrown when there are conflicting frames in the VMD data. |
| `FrameOverflowError` | Exception thrown when the number of frames exceeds the maximum allowed by the library. |
| `IncompatibleFormatError` | Exception thrown when the file format is incompatible with the expected format. |
| `InvalidFieldValueError` | Exception thrown when a field in the frame data structures contains an invalid value. |
| `StringProcessError` | Exception thrown when there is an error in encoding conversion. |

<br>These exceptions may be thrown by `readVMD()`, `writeVMD()`, `VMDString` conversion functions, or encoding conversion functions when errors occur during processing.