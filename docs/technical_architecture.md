## Technical Architecture

### 1. Overview
VMD (Vocaloid Motion Data) files are divided into two types of data, which are strictly distinguished by MMD during import—mismatched data is rejected depending on whether the active mode is "Model Edit" or "Camera Edit". Thus, the library is designed to reflect this strict format separation in its architecture.

There are two main namespaces in the library that correspond to these two types of VMD data, as well as additional namespaces for encoding conversion and exception handling:


| Namespace | Header file |  Description |
| --- | --- | --- |
| `vmdio::camera_edit` | `camera_edit.h` | Data structures and functions for camera edit VMD files. |
| `vmdio::model_edit` | `model_edit.h` | Data structures and functions for model edit VMD files. |
| `vmdio::encoding` | `encoding.h` | String encoding conversion functions. |
| `vmdio::exceptions` | `exceptions.h` | Custom exception classes for error handling. |

<br>

You can choose to include only the header files for the namespace that corresponds to the type of VMD data you want to work with. Optionally, you can also include the encoding conversion header and the exceptions header if you want to use the encoding conversion functions and custom exceptions in your code.

Basic usage in C++ would look like this:

```cpp
// Working with model edit VMD files 

#include <vmdio/model_edit.h>
#include <vmdio/exceptions.h>

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

`readVMD()` interprets the binary data from a VMD file and populates a `VMDData` structure with more user-friendly data structures.

```cpp
VMDData readVMD(const std::filesystem::path &pFilePath);
```

<br>`writeVMD()` takes a `VMDData` structure and serializes it back into the binary VMD file format.

```cpp
void writeVMD(const VMDData &pVmdData, const std::filesystem::path &pFilePath);
```

<br>During reading and writing, custom exceptions defined in vmdio::exceptions may be thrown for errors such as file I/O failures, incompatible formats, invalid field values, string conversion failures, or total frame counts exceeding the supported limit.


`readVMD()` preserves values from the VMD file as faithfully as possible when they can be represented by the library's data structures. It throws exceptions for values that cannot be represented by those structures, such as undefined enum-backed flag values, or for fields whose string decoding fails.

`writeVMD()` validates data before serialization and throws exceptions when invalid values are detected or when string encoding fails. The current validation rules are as follows:

- `vmdio::model_edit`
  - empty model name
  - duplicate `MotionFrame` entries (same `boneName` and same `frameNumber`)
  - duplicate `MorphFrame` entries (same `morphName` and same `frameNumber`)
  - duplicate `VisibleIKFrame` entries (same `frameNumber`)
  - empty name fields in `MotionFrame` / `MorphFrame` / `IKData`
  - invalid values of `Visibility` / `IKState`
  - norm of `MotionFrame::rotation` is close to zero
  - out-of-range values in `ControlPointSet`

- `vmdio::camera_edit`
  - duplicate `CameraFrame` / `LightFrame` / `SelfShadowFrame` entries (same `frameNumber`)
  - invalid values of `ProjectionType` / `SelfShadowMode`
  - out-of-range values in `ControlPointSet`

<br>


If `MotionFrame::rotation` is non-zero but not unit length, it is normalized internally during serialization. `MorphFrame::value`, `CameraFrame::viewingAngle`, `LightFrame::color`, and `SelfShadowFrame::shadowRange` are not validated against the usual MMD UI slider ranges, since MMD can still read and write values outside those ranges when entered directly.

<br>

### 3. VMD Data Structure
The `VMDData` structure serves as the main container for all the data read from or to be written to a VMD file, holding the vector of key frame data structures for each type of frame:

```cpp
// vmdio::model_edit::VMDData structure definition

struct VMDData
{
    std::string modelName;
    std::vector<MotionFrame> motionFrames;
    std::vector<MorphFrame> morphFrames;
    std::vector<VisibleIKFrame> visibleIKFrames;
};
```

<br>This structure is also defined in both `vmdio::camera_edit` and `vmdio::model_edit` namespaces, but the fields are different to reflect the different types of data contained in camera edit and model edit VMD files.

Note that `VMDData` structure does not represent all the raw data fields in the VMD file format. The headers and model name (in camera edit VMD files) fields are not exposed as public API since the library internally handles them during reading and writing. 

<br>

### 4. Key Frame Data Structures

The following are all key frame data structures defined in the library:

|Namespace| Defined Frame Data Structures |
| --- | --- |
| `vmdio::camera_edit` | `CameraFrame`, `LightFrame`, `SelfShadowFrame` |
| `vmdio::model_edit` | `MotionFrame`, `MorphFrame`, `VisibleIKFrame` |

<br>

Each frame data structure contains fields that represent the keyframe data for that particular type of frame:

```cpp
// vmdio::model_edit::MotionFrame structure definition
struct MotionFrame
{
    std::string boneName;
    uint32_t frameNumber = 0;
    Position position;
    Quaternion rotation;
    MotionInterpolation interpolation;
};

// vmdio::camera_edit::CameraFrame structure definition
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

<br>

Note that all frame data structures are initialized with default values for their fields or default values for custom types of fields except string fields, which are initialized to empty strings. All these "default values" are set to the same values as UI default values in MMD.

<br>

### 5. Value Data Types and Structures
The following are the value types and structures defined in the library:

|Namespace| Defined Value Data types and Structures |
| --- | --- |
| `vmdio::camera_edit` | `ProjectionType`, `SelfShadowMode`, `camera_edit::Position`, `CameraRotation`, `camera_edit::ControlPointSet`, `CameraInterpolation`, `Color` |
| `vmdio::model_edit` | `IKState`, `Visibility`, `model_edit::Position`, `Quaternion`, `model_edit::ControlPointSet`, `MotionInterpolation` |

<br>These are designed to be more user-friendly compared to the raw byte-level representation in the VMD file format. For example:

- Enum class for flag fields instead of raw byte
    ```cpp
    enum class IKState : uint8_t
    {
        OFF = 0, 
        ON = 1   
    };
    ```

- Clear semantic types for numerical data instead of raw float arrays
    ```cpp
    struct Position
    {
        float x;
        float y;
        float z;
    };
    ```

- Simplified interpolation data representation instead of long byte arrays
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

<br> Although `Position` and `ControlPointSet` have identical definitions in both namespaces, they are defined separately in order to preserve the architectural separation between them.

For string fields, the public API of this library represents them as `UTF-8` encoded `std::string` values. In contrast, string fields in VMD files are fixed-length `Shift_JIS` fields, and the required encoding conversions are performed internally by the library.

When writing, the library converts strings from `UTF-8` to `Shift_JIS` and throws an exception if the converted byte length exceeds the field size limit or if the string contains characters that cannot be converted. The limits are as follows:

| Field | Byte Length Limit (after Shift_JIS conversion) |
| ----- | ---------------------------------------------- |
| `vmdio::model_edit::VMDData::modelName` | 20 bytes |
| `vmdio::model_edit::MotionFrame::boneName` | 15 bytes |
| `vmdio::model_edit::MorphFrame::morphName` | 15 bytes |
| `vmdio::model_edit::IKData::ikBoneName` | 20 bytes |

<br>Likewise, when reading, the library converts strings from `Shift_JIS` to `UTF-8` and throws an exception if decoding fails.

<br>

### 6. Encoding Conversion
Including the `encoding.h` header allows you to use the encoding conversion functions defined in the `vmdio::encoding` namespace:

```cpp
std::string utf8ToShiftJIS(std::string_view pString);

std::string shiftJISToUTF8(std::string_view pString);
```

<br>

These functions are originally designed for internal use in the library to handle the encoding conversions when reading and writing VMD files, but they can also be used by users of the library if needed for their own string encoding conversion purposes.


<br>

### 7. Custom Exceptions
The `exceptions.h` defines the following custom exception classes for error handling in the library:

| Exception Class | Brief Description |
| --- | --- |
| `VMDIOError` | Base class for all exceptions in the library. |
| `FileSystemError` | Exception related to file I/O operations. |
| `FrameConflictError` | Exception thrown when there are conflicting frames in the VMD data. |
| `FrameOverflowError` | Exception thrown when the number of frames exceeds the maximum allowed by the MMD. |
| `IncompatibleFormatError` | Exception thrown when the file format is incompatible with the expected format. |
| `InvalidFieldValueError` | Exception thrown when a field in the frame data structures contains an invalid value. |
| `StringProcessError` | Exception thrown when there is an error in encoding conversion. |

<br>These exceptions may be thrown by the `readVMD()` and `writeVMD()` functions when errors occur during the reading and writing processes.