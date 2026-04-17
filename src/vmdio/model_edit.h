#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

/**
 * @namespace vmdio::model_edit
 * @brief Namespace for all data structures and functions related to model, morph, and visible IK
 *        motion data.
 */
namespace vmdio::model_edit
{
    /**
     * @enum IKState
     * @brief Enumeration for IK state flags in the IK data of the VisibleIKFrame.
     */
    enum class IKState : uint8_t
    {
        OFF = 0, ///< IK is OFF
        ON = 1   ///< IK is ON
    };

    /**
     * @enum Visibility
     * @brief Enumeration for visibility flags in the VisibleIKFrame.
     */
    enum class Visibility : uint8_t
    {
        Hidden = 0, ///< Visibility is OFF
        Visible = 1 ///< Visibility is ON
    };

    /**
     * @struct Position
     * @brief Struct representing a 3D position with x, y, and z coordinates.
     * @details The coordinates are stored as floats and represent the local position in DirectX
     *          coordinate system (Left-handed Cartesian Coordinates).
     * @note This struct is also defined in camera_edit namespace, but is duplicated here to avoid
     *       unnecessary dependencies between model_edit and camera_edit namespaces.
     */
    struct Position
    {
        float x = 0.0f; ///< X coordinate
        float y = 0.0f; ///< Y coordinate
        float z = 0.0f; ///< Z coordinate
    };

    /**
     * @struct Quaternion
     * @brief Struct representing a rotation using quaternion components qx, qy, qz, and qw.
     * @details The quaternion is stored as four floats and represents the rotation in DirectX
     *          coordinate system (Left-handed Cartesian Coordinates).
     * @note For MotionFrame::rotation, a unit quaternion is recommended.
     *       model_edit::writeVMD() treats a quaternion whose norm is close to zero as invalid and
     *       throws an exception. If the quaternion is non-zero but not normalized, it is normalized
     *       internally during serialization.
     */
    struct Quaternion
    {
        float qx = 0.0f; ///< X component of the vector part of the quaternion
        float qy = 0.0f; ///< Y component of the vector part of the quaternion
        float qz = 0.0f; ///< Z component of the vector part of the quaternion
        float qw = 1.0f; ///< Scalar part of the quaternion
    };

    /**
     * @struct ControlPointSet
     * @brief Struct representing a set of control points (x1, y1) and (x2, y2).
     * @details This struct is used for interpolation control points in the
     *          model_edit::MotionInterpolation struct. The default values are set to represent
     *          the default interpolation curve used in MMD, which is linear interpolation.
     * @note This struct is also defined in camera_edit namespace, but is duplicated here to avoid
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
     * @struct MotionInterpolation
     * @brief Struct representing the interpolation control points for motion frames.
     * @note The interpolation is applied when transitioning into the frame.
     */
    struct MotionInterpolation
    {
        ControlPointSet xPos; ///< X position interpolation
        ControlPointSet yPos; ///< Y position interpolation
        ControlPointSet zPos; ///< Z position interpolation
        ControlPointSet rot;  ///< Rotation interpolation
    };

    /**
     * @struct MotionFrame
     * @brief Struct representing a single keyframe for motion data in the VMD file.
     */
    struct MotionFrame
    {
        ///< Name of the bone
        std::string boneName;

        ///< Frame number
        uint32_t frameNumber = 0;

        ///< Position of the bone
        Position position;

        ///< Quaternion rotation of the bone.
        ///< A non-zero non-unit quaternion is normalized when written to a VMD file.
        Quaternion rotation;

        ///< Interpolation control points for the motion frame
        MotionInterpolation interpolation;
    };

    /**
     * @struct MorphFrame
     * @brief Struct representing a single keyframe for morph data in the VMD file.
     * @note The morph value is typically in the range of 0.0 to 1.0 for most morphs,
     *       but this library does not enforce that range and serializes the stored
     *       value as-is, since MMD can read and write values outside that range
     *       when entered directly.
     */
    struct MorphFrame
    {
        ///< Name of the morph
        std::string morphName;

        ///< Frame number
        uint32_t frameNumber = 0;

        ///< Value of the morph
        float value = 0.0f;
    };

    /**
     * @struct IKData
     * @brief Struct representing the IK data stored in the VisibleIKFrame.
     */
    struct IKData
    {
        ///< Name of the IK bone
        std::string ikBoneName;

        ///< IK state (default: ON)
        IKState ikState = IKState::ON;
    };

    /**
     * @struct VisibleIKFrame
     * @brief Struct representing a single keyframe for visible IK data in the VMD file.
     */
    struct VisibleIKFrame
    {
        ///< Frame number
        uint32_t frameNumber = 0;

        ///< Visibility flag
        Visibility visibility = Visibility::Visible;

        ///< List of IK data for the frame
        std::vector<IKData> ikDataList;
    };

    /**
     * @struct VMDData
     * @brief Struct representing the content of a model edit VMD file.
     * @note All string fields in this class are stored in UTF-8 encoding. model_edit::readVMD and
     *       model_edit::writeVMD functions will handle the conversion between UTF-8 and Shift_JIS
     *       encoding when reading from and writing to VMD files.
     */
    struct VMDData
    {
        ///< Name of the model
        std::string modelName;

        ///< List of motion frames
        std::vector<MotionFrame> motionFrames;

        ///< List of morph frames
        std::vector<MorphFrame> morphFrames;

        ///< List of visible IK frames
        std::vector<VisibleIKFrame> visibleIKFrames;
    };

    /**
     * @brief Reads a VMD file and returns a VMDData structure.
     * @returns A VMDData structure containing the data read from the specified VMD file.
     * @param pFilePath The path to the VMD file to read.
     * @throws vmdio::exceptions::FileSystemError If the file does not exist, cannot be opened, or
     *         an I/O error occurs.
     * @throws vmdio::exceptions::IncompatibleFormatError If the file extension is not .vmd,
     *         the header is invalid, the file is for camera edit instead of model edit, or
     *         the file ends unexpectedly.
     * @throws vmdio::exceptions::InvalidFieldValueError If the file contains invalid data
     *         (e.g., out-of-range flags or enum values).
     * @throws vmdio::exceptions::StringProcessError If there is an error during encoding
     *         conversion of string fields.
     * @note This function will populate string fields in pVmdData with UTF-8 encoded strings.
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
     *         the model name is the same as the camera edit VMD model name, or the file ends
     *         unexpectedly.
     * @throws vmdio::exceptions::InvalidFieldValueError If model name is empty or any frame
     *         contains invalid data (e.g., out-of-range values).
     * @throws vmdio::exceptions::StringProcessError If there is an error during encoding
     *         conversion of string fields.
     * @note This function will overwrite the file if it already exists.
     * @note This function will write string data from pVmdData in Shift_JIS encoding.
     * @note For model_edit::MotionFrame::rotation, if the quaternion is non-zero but not
     *       normalized, this function normalizes it internally during serialization.
     *       The input VMDData object itself is not modified.
     */
    void writeVMD(const VMDData &pVmdData, const std::filesystem::path &pFilePath);
}