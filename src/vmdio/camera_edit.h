#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

/**
 * @namespace vmdio::camera_edit
 * @brief Namespace for all data structures and functions related to camera, light, and self shadow
 *        motion data.
 */
namespace vmdio::camera_edit
{
    /**
     * @enum ProjectionType
     * @brief Enumeration for camera projection type flags in the CameraFrame.
     */
    enum class ProjectionType : uint8_t
    {
        Perspective = 0, ///< Perspective projection
        Orthographic = 1 ///< Orthographic projection
    };

    /**
     * @enum SelfShadowMode
     * @brief Enumeration for self shadow mode flags in the SelfShadowFrame.
     */
    enum class SelfShadowMode : uint8_t
    {
        NoSelfShadow = 0, ///< No self shadow
        Mode1 = 1,        ///< Self shadow mode 1
        Mode2 = 2         ///< Self shadow mode 2
    };

    /**
     * @struct Position
     * @brief Struct representing a 3D position with x, y, and z coordinates.
     * @details The coordinates are stored as floats and represent the local position in DirectX
     *          coordinate system (Left-handed Cartesian Coordinates).
     * @note This struct is also defined in model_edit namespace, but is duplicated here to avoid
     *       unnecessary dependencies between model_edit and camera_edit namespaces.
     */
    struct Position
    {
        float x = 0.0f; ///< X coordinate
        float y = 0.0f; ///< Y coordinate
        float z = 0.0f; ///< Z coordinate
    };

    /**
     * @struct CameraRotation
     * @brief Struct representing the camera rotation in Euler angles (eulerX, eulerY, eulerZ).
     * @details The Euler angles are stored as degrees and represent the rotation in DirectX
     *          coordinate system (Left-handed Cartesian Coordinates).
     * @note The Euler angles are stored in the VMD file as follows:
     *       - eulerX is stored as "(eulerX) * pi / 180.0 * (-1.0)"
     *       - eulerY is stored as "(eulerY) * pi / 180.0"
     *       - eulerZ is stored as "(eulerZ) * pi / 180.0"
     */
    struct CameraRotation
    {
        float eulerX = 0.0f; ///< X rotation
        float eulerY = 0.0f; ///< Y rotation
        float eulerZ = 0.0f; ///< Z rotation
    };

    /**
     * @struct ControlPointSet
     * @brief Struct representing a set of control points (x1, y1) and (x2, y2).
     * @details This struct is used for interpolation control points in the
     *          camera_edit::CameraInterpolation struct. The default values are set to represent
     *          the default interpolation curve used in MMD, which is linear interpolation.
     * @note This struct is also defined in model_edit namespace, but is duplicated here to avoid
     *       unnecessary dependencies between model_edit and camera_edit namespaces.
     */
    struct ControlPointSet
    {
        uint32_t x1 = 20;  ///< X coordinate of the first control point (0 to 127)
        uint32_t y1 = 20;  ///< Y coordinate of the first control point (0 to 127)
        uint32_t x2 = 107; ///< X coordinate of the second control point (0 to 127)
        uint32_t y2 = 107; ///< Y coordinate of the second control point (0 to 127)
    };

    /**
     * @struct CameraInterpolation
     * @brief Struct representing the interpolation control points for camera frames.
     * @note The interpolation is applied when transitioning into the frame.
     */
    struct CameraInterpolation
    {
        ControlPointSet xPos; ///< X position
        ControlPointSet yPos; ///< Y position
        ControlPointSet zPos; ///< Z position
        ControlPointSet rot;  ///< Rotation
        ControlPointSet dist; ///< Distance
        ControlPointSet view; ///< Viewing angle
    };

    /**
     * @struct Color
     * @brief Struct representing a color with R, G, and B components.
     * @details The color components are stored as "(Color::element) / 256.0" in the VMD file.
     * @note In the MMD UI, color components are typically displayed in the range 0 to 255.
     *       However, this library does not enforce that UI range and serializes the stored
     *       values as-is.
     */
    struct Color
    {
        int32_t r = 0; ///< Red component
        int32_t g = 0; ///< Green component
        int32_t b = 0; ///< Blue component
    };

    /**
     * @struct CameraFrame
     * @brief Struct representing a single keyframe for camera data in the VMD file.
     * @note The distance value is stored as "(UI Displayed Distance) * (-1.0)" in the VMD file.
     * @note viewingAngle is represented as an integer value. Although the MMD UI commonly presents
     *       a usual adjustment range, this library does not enforce that UI range and serializes
     *       the stored value as-is.
     */
    struct CameraFrame
    {
        ///< Frame number
        uint32_t frameNumber = 0;

        ///< Distance of the camera
        float distance = 45.00f;

        ///< Position of the camera
        Position position;

        ///< Rotation of the camera
        CameraRotation rotation;

        ///< Interpolation control points for the camera frame
        CameraInterpolation interpolation;

        ///< Viewing angle of the camera
        int32_t viewingAngle = 30;

        ///< Projection type of the camera
        ProjectionType projectionType = ProjectionType::Perspective;
    };

    /**
     * @struct LightFrame
     * @brief Struct representing a single keyframe for light data in the VMD file.
     */
    struct LightFrame
    {
        ///< Frame number
        uint32_t frameNumber = 0;

        ///< Color of the light
        Color color{154, 154, 154};

        ///< Position of the light
        Position position{-0.5f, -1.0f, 0.5f};
    };

    /**
     * @struct SelfShadowFrame
     * @brief Struct representing a single keyframe for self shadow data in the VMD file.
     * @note The shadow range value is stored as "0.1f - (UI Displayed Distance) * 0.00001f"
     *       in the VMD file.
     * @note Although the MMD UI commonly presents a usual adjustment range for this value,
     *       this library does not enforce that UI range and serializes the stored integer
     *       value as-is.
     */
    struct SelfShadowFrame
    {

        ///< Frame number
        uint32_t frameNumber = 0;

        ///< Self shadow mode
        SelfShadowMode mode = SelfShadowMode::Mode1;

        ///< Shadow range value
        int32_t shadowRange = 8875;
    };

    /**
     * @struct VMDData
     * @brief Struct representing the content of a camera edit VMD file.
     */
    struct VMDData
    {
        ///< List of camera frames
        std::vector<CameraFrame> cameraFrames;

        ///< List of light frames
        std::vector<LightFrame> lightFrames;

        ///< List of self shadow frames
        std::vector<SelfShadowFrame> selfShadowFrames;
    };

    /**
     * @brief Reads a VMD file and returns a VMDData structure.
     * @return A VMDData structure containing the data read from the specified VMD file.
     * @param pFilePath The path to the VMD file to read.
     * @throws vmdio::exceptions::FileSystemError If the file does not exist, cannot be opened,
     *         or an I/O error occurs.
     * @throws vmdio::exceptions::FrameOverflowError If the file contains more frames than the
     *         maximum allowed limit.
     * @throws vmdio::exceptions::IncompatibleFormatError If the file extension is not .vmd,
     *         the header is invalid, the file is for model edit instead of camera edit, or
     *         the file ends unexpectedly.
     * @throws vmdio::exceptions::InvalidFieldValueError If the file contains invalid data
     *         (e.g., out-of-range flags or enum values).
     * @throws vmdio::exceptions::StringProcessError If there is an error during encoding
     *         conversion of string fields.
     */
    VMDData readVMD(const std::filesystem::path &pFilePath);

    /**
     * @brief Writes the provided VMDData data to a VMD file.
     * @param pVmdData The VMDData structure containing the data to write.
     * @param pFilePath The path to the VMD file to write to.
     * @throws vmdio::exceptions::FileSystemError If the file cannot be opened for writing or
     *         an I/O error occurs.
     * @throws vmdio::exceptions::FrameOverflowError If the total number of frames exceeds
     *         the maximum allowed limit.
     * @throws vmdio::exceptions::FrameConflictError If there are duplicate frames for the same
     *         target at the same frame number.
     * @throws vmdio::exceptions::IncompatibleFormatError If the file extension is not .vmd,
     *         or the file ends unexpectedly.
     * @throws vmdio::exceptions::InvalidFieldValueError If any frame contains invalid data
     *         (e.g., out-of-range values).
     * @throws vmdio::exceptions::StringProcessError If there is an error during encoding
     *         conversion of string fields.
     * @note This function will overwrite the file if it already exists.
     */
    void writeVMD(const VMDData &pVmdData, const std::filesystem::path &pFilePath);
}