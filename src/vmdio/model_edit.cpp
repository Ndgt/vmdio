#include "model_edit.h"

#include <set>
#include <string_view>
#include <tuple>
#include <unordered_set>

#include "encoding.h"
#include "exceptions.h"
#include "private/vmd_constants.h"
#include "private/vmd_io_utils.h"

namespace
{
    inline void validateModelName(const vmdio::model_edit::VMDData &pVmdData)
    {
        if (pVmdData.modelName == vmdio::internal::VMD_MODEL_NAME_CAMERA_EDIT)
        {
            throw vmdio::exceptions::IncompatibleFormatError(
                "The model name must not be the same as the camera edit VMD model name. Model name:" + pVmdData.modelName);
        }

        if (pVmdData.modelName.empty())
        {
            throw vmdio::exceptions::InvalidFieldValueError(
                "Model name in the VMD data cannot be empty.");
        }
    }

    inline void validateFrameCount(const vmdio::model_edit::VMDData &pVmdData)
    {
        size_t lTotalFrames = pVmdData.motionFrames.size() + pVmdData.morphFrames.size() +
                              pVmdData.visibleIKFrames.size();

        if (lTotalFrames > vmdio::internal::MAX_FRAME_COUNT)
        {
            throw vmdio::exceptions::FrameOverflowError(
                "Total frame count in the VMD data exceeds the supported maximum limit of " + std::to_string(vmdio::internal::MAX_FRAME_COUNT) +
                ". Total frames: " + std::to_string(lTotalFrames));
        }
    }

    inline void validateRotation(
        const vmdio::model_edit::Quaternion &pRotation, uint32_t pFrameNumber)
    {
        float lNormSquared = pRotation.qx * pRotation.qx + pRotation.qy * pRotation.qy +
                             pRotation.qz * pRotation.qz + pRotation.qw * pRotation.qw;

        // Throw an error if the norm of the quaternion is close to zero
        if (lNormSquared < 1e-5f)
        {
            throw vmdio::exceptions::InvalidFieldValueError(
                "Rotation quaternion in the motion frame must not be a zero quaternion. Frame number: " + std::to_string(pFrameNumber) +
                " Quaternion: (" +
                std::to_string(pRotation.qx) + ", " +
                std::to_string(pRotation.qy) + ", " +
                std::to_string(pRotation.qz) + ", " +
                std::to_string(pRotation.qw) + ")");
        }
    }

    inline void validateMotionInterpolation(
        const vmdio::model_edit::MotionInterpolation &pInterpolation, uint32_t pFrameNumber)
    {
        vmdio::internal::validateControlPointSet(
            pInterpolation.xPos, "MotionInterpolation.xPos", pFrameNumber);
        vmdio::internal::validateControlPointSet(
            pInterpolation.yPos, "MotionInterpolation.yPos", pFrameNumber);
        vmdio::internal::validateControlPointSet(
            pInterpolation.zPos, "MotionInterpolation.zPos", pFrameNumber);
        vmdio::internal::validateControlPointSet(
            pInterpolation.rot, "MotionInterpolation.rot", pFrameNumber);
    }

    inline void validateMotionFrames(const vmdio::model_edit::VMDData &pVmdData)
    {
        std::set<std::tuple<uint32_t, std::string>> lFrameNumberBoneNameSet;

        for (const auto &lMotionFrame : pVmdData.motionFrames)
        {
            if (lMotionFrame.boneName.empty())
            {
                throw vmdio::exceptions::InvalidFieldValueError(
                    "Bone name in the motion frame cannot be empty. Frame number: " +
                    std::to_string(lMotionFrame.frameNumber));
            }

            auto [it, lInserted] =
                lFrameNumberBoneNameSet.insert({lMotionFrame.frameNumber, lMotionFrame.boneName});

            if (!lInserted)
            {
                throw vmdio::exceptions::FrameConflictError(
                    "Duplicate motion frame detected with bone name '" + lMotionFrame.boneName +
                    "' and frame number " + std::to_string(lMotionFrame.frameNumber));
            }

            validateRotation(lMotionFrame.rotation, lMotionFrame.frameNumber);
            validateMotionInterpolation(lMotionFrame.interpolation, lMotionFrame.frameNumber);
        }
    }

    inline void validateMorphFrames(const vmdio::model_edit::VMDData &pVmdData)
    {
        std::set<std::tuple<uint32_t, std::string>> lFrameNumberMorphNameSet;

        for (const auto &lMorphFrame : pVmdData.morphFrames)
        {
            if (lMorphFrame.morphName.empty())
            {
                throw vmdio::exceptions::InvalidFieldValueError(
                    "Morph name in the morph frame cannot be empty. Frame number: " +
                    std::to_string(lMorphFrame.frameNumber));
            }

            auto [it, lInserted] =
                lFrameNumberMorphNameSet.insert({lMorphFrame.frameNumber, lMorphFrame.morphName});

            if (!lInserted)
            {
                throw vmdio::exceptions::FrameConflictError(
                    "Duplicate morph frame detected with morph name '" + lMorphFrame.morphName +
                    "' and frame number " + std::to_string(lMorphFrame.frameNumber));
            }
        }
    }

    inline void validateVisibleIKFrames(const vmdio::model_edit::VMDData &pVmdData)
    {
        std::unordered_set<uint32_t> lFrameNumberSet;

        for (const auto &lVisibleIKFrame : pVmdData.visibleIKFrames)
        {
            if (!lFrameNumberSet.insert(lVisibleIKFrame.frameNumber).second)
            {
                throw vmdio::exceptions::FrameConflictError(
                    "Duplicate visible IK frame detected with frame number " +
                    std::to_string(lVisibleIKFrame.frameNumber));
            }

            if (lVisibleIKFrame.visibility != vmdio::model_edit::Visibility::Hidden &&
                lVisibleIKFrame.visibility != vmdio::model_edit::Visibility::Visible)
            {
                throw vmdio::exceptions::InvalidFieldValueError(
                    "Visibility in the visible IK frame must be either Hidden (0) or Visible (1). Frame number: " + std::to_string(lVisibleIKFrame.frameNumber) +
                    " Invalid visibility value: " +
                    std::to_string(static_cast<uint32_t>(lVisibleIKFrame.visibility)));
            }

            for (const auto &lIKData : lVisibleIKFrame.ikDataList)
            {
                if (lIKData.ikBoneName.empty())
                {
                    throw vmdio::exceptions::InvalidFieldValueError(
                        "IK bone name in the visible IK frame cannot be empty. Frame number: " + std::to_string(lVisibleIKFrame.frameNumber));
                }

                if (lIKData.ikState != vmdio::model_edit::IKState::OFF &&
                    lIKData.ikState != vmdio::model_edit::IKState::ON)
                {
                    throw vmdio::exceptions::InvalidFieldValueError(
                        "IK state in the visible IK frame must be either OFF (0) or ON (1). Frame number: " + std::to_string(lVisibleIKFrame.frameNumber) +
                        " Invalid IK state: " +
                        std::to_string(static_cast<uint32_t>(lIKData.ikState)));
                }
            }
        }
    }

    inline float getQuaternionNorm(const vmdio::model_edit::Quaternion &pQ)
    {
        return std::sqrt(pQ.qx * pQ.qx + pQ.qy * pQ.qy + pQ.qz * pQ.qz + pQ.qw * pQ.qw);
    }
}

namespace vmdio::model_edit
{
    void readMotionInterpolation(std::ifstream &pStream, MotionInterpolation &pInterpolation)
    {
        // Read interpolation
        uint8_t lInterpBuffer[internal::VMD_MOTION_INTERPOLATION_FIELD_SIZE];
        internal::readInterpolationBuffer(pStream, lInterpBuffer, sizeof(lInterpBuffer));

        pInterpolation.xPos.x1 = static_cast<uint32_t>(lInterpBuffer[0]);
        pInterpolation.xPos.y1 = static_cast<uint32_t>(lInterpBuffer[4]);
        pInterpolation.xPos.x2 = static_cast<uint32_t>(lInterpBuffer[8]);
        pInterpolation.xPos.y2 = static_cast<uint32_t>(lInterpBuffer[12]);

        pInterpolation.yPos.x1 = static_cast<uint32_t>(lInterpBuffer[16]);
        pInterpolation.yPos.y1 = static_cast<uint32_t>(lInterpBuffer[20]);
        pInterpolation.yPos.x2 = static_cast<uint32_t>(lInterpBuffer[24]);
        pInterpolation.yPos.y2 = static_cast<uint32_t>(lInterpBuffer[28]);

        pInterpolation.zPos.x1 = static_cast<uint32_t>(lInterpBuffer[32]);
        pInterpolation.zPos.y1 = static_cast<uint32_t>(lInterpBuffer[36]);
        pInterpolation.zPos.x2 = static_cast<uint32_t>(lInterpBuffer[40]);
        pInterpolation.zPos.y2 = static_cast<uint32_t>(lInterpBuffer[44]);

        pInterpolation.rot.x1 = static_cast<uint32_t>(lInterpBuffer[48]);
        pInterpolation.rot.y1 = static_cast<uint32_t>(lInterpBuffer[52]);
        pInterpolation.rot.x2 = static_cast<uint32_t>(lInterpBuffer[56]);
        pInterpolation.rot.y2 = static_cast<uint32_t>(lInterpBuffer[60]);
    }

    void writeMotionInterpolation(std::ofstream &pStream, const MotionInterpolation &pInterpolation)
    {
        // Initialize buffer with zeros
        uint8_t lInterpBuffer[internal::VMD_MOTION_INTERPOLATION_FIELD_SIZE] = {};

        lInterpBuffer[0] = static_cast<uint8_t>(pInterpolation.xPos.x1);
        lInterpBuffer[4] = static_cast<uint8_t>(pInterpolation.xPos.y1);
        lInterpBuffer[8] = static_cast<uint8_t>(pInterpolation.xPos.x2);
        lInterpBuffer[12] = static_cast<uint8_t>(pInterpolation.xPos.y2);

        lInterpBuffer[16] = static_cast<uint8_t>(pInterpolation.yPos.x1);
        lInterpBuffer[20] = static_cast<uint8_t>(pInterpolation.yPos.y1);
        lInterpBuffer[24] = static_cast<uint8_t>(pInterpolation.yPos.x2);
        lInterpBuffer[28] = static_cast<uint8_t>(pInterpolation.yPos.y2);

        lInterpBuffer[32] = static_cast<uint8_t>(pInterpolation.zPos.x1);
        lInterpBuffer[36] = static_cast<uint8_t>(pInterpolation.zPos.y1);
        lInterpBuffer[40] = static_cast<uint8_t>(pInterpolation.zPos.x2);
        lInterpBuffer[44] = static_cast<uint8_t>(pInterpolation.zPos.y2);

        lInterpBuffer[48] = static_cast<uint8_t>(pInterpolation.rot.x1);
        lInterpBuffer[52] = static_cast<uint8_t>(pInterpolation.rot.y1);
        lInterpBuffer[56] = static_cast<uint8_t>(pInterpolation.rot.x2);
        lInterpBuffer[60] = static_cast<uint8_t>(pInterpolation.rot.y2);

        // NOTE: The remaining 48 bytes seem to be unused in MMD.

        internal::writeInterpolationBuffer(pStream, lInterpBuffer, sizeof(lInterpBuffer));
    }

    void readMotionFrame(std::ifstream &pStream, MotionFrame &pMotionFrame)
    {
        // Read bone name
        pMotionFrame.boneName = internal::readStringField(
            pStream, internal::VMD_BONE_NAME_FIELD_SIZE, "Bone name");

        // Read frame number
        internal::readUintValue(pStream, pMotionFrame.frameNumber);

        // Read position
        internal::readFloatValue(pStream, pMotionFrame.position.x);
        internal::readFloatValue(pStream, pMotionFrame.position.y);
        internal::readFloatValue(pStream, pMotionFrame.position.z);

        // Read rotation
        internal::readFloatValue(pStream, pMotionFrame.rotation.qx);
        internal::readFloatValue(pStream, pMotionFrame.rotation.qy);
        internal::readFloatValue(pStream, pMotionFrame.rotation.qz);
        internal::readFloatValue(pStream, pMotionFrame.rotation.qw);

        // Read interpolation
        readMotionInterpolation(pStream, pMotionFrame.interpolation);
    }

    void writeMotionFrame(std::ofstream &pStream, const MotionFrame &pMotionFrame)
    {
        // Write bone name
        internal::writeStringField(
            pStream, pMotionFrame.boneName, internal::VMD_BONE_NAME_FIELD_SIZE, "Bone name");

        // Write frame number
        internal::writeUintValue(pStream, pMotionFrame.frameNumber);

        // Write position
        internal::writeFloatValue(pStream, pMotionFrame.position.x);
        internal::writeFloatValue(pStream, pMotionFrame.position.y);
        internal::writeFloatValue(pStream, pMotionFrame.position.z);

        // Get quaternion norm before writing rotation
        float lQNorm = getQuaternionNorm(pMotionFrame.rotation);

        // Write rotation
        if (std::abs(1 - lQNorm) < 0.01f)
        {
            internal::writeFloatValue(pStream, pMotionFrame.rotation.qx);
            internal::writeFloatValue(pStream, pMotionFrame.rotation.qy);
            internal::writeFloatValue(pStream, pMotionFrame.rotation.qz);
            internal::writeFloatValue(pStream, pMotionFrame.rotation.qw);
        }
        else
        {
            // Write rotation with normalized quaternion
            internal::writeFloatValue(pStream, pMotionFrame.rotation.qx / lQNorm);
            internal::writeFloatValue(pStream, pMotionFrame.rotation.qy / lQNorm);
            internal::writeFloatValue(pStream, pMotionFrame.rotation.qz / lQNorm);
            internal::writeFloatValue(pStream, pMotionFrame.rotation.qw / lQNorm);
        }

        // Write interpolation
        writeMotionInterpolation(pStream, pMotionFrame.interpolation);
    }

    void readMorphFrame(std::ifstream &pStream, MorphFrame &pMorphFrame)
    {
        // Read morph name
        pMorphFrame.morphName = internal::readStringField(
            pStream, internal::VMD_MORPH_NAME_FIELD_SIZE, "Morph name");

        // Read frame number
        internal::readUintValue(pStream, pMorphFrame.frameNumber);

        // Read morph value (float)
        internal::readFloatValue(pStream, pMorphFrame.value);
    }

    void writeMorphFrame(std::ofstream &pStream, const MorphFrame &pMorphFrame)
    {
        // Write morph name
        internal::writeStringField(
            pStream, pMorphFrame.morphName, internal::VMD_MORPH_NAME_FIELD_SIZE, "Morph name");

        // Write frame number
        internal::writeUintValue(pStream, pMorphFrame.frameNumber);

        // Write morph value
        internal::writeFloatValue(pStream, pMorphFrame.value);
    }

    void readVisibleIKFrame(std::ifstream &pStream, VisibleIKFrame &pVisibleIKFrame)
    {
        // Read frame number
        internal::readUintValue(pStream, pVisibleIKFrame.frameNumber);

        // Read visible flag
        uint8_t lVisibleFlag;
        internal::readFlagValue(pStream, lVisibleFlag);

        if (lVisibleFlag == 0)
            pVisibleIKFrame.visibility = Visibility::Hidden;
        else if (lVisibleFlag == 1)
            pVisibleIKFrame.visibility = Visibility::Visible;
        else
        {
            throw exceptions::InvalidFieldValueError(
                "Visibility flag in the visible IK frame must be either 0 (Hidden) or 1 (Visible)."
                "Frame number: " +
                std::to_string(pVisibleIKFrame.frameNumber) +
                " Invalid visibility flag: " +
                std::to_string(lVisibleFlag));
        }

        // Read IK data count
        uint32_t lIKDataCount;
        internal::readUintValue(pStream, lIKDataCount);

        // Clear and reserve IK data list
        pVisibleIKFrame.ikDataList.clear();
        pVisibleIKFrame.ikDataList.reserve(lIKDataCount);

        // Read IK data list
        for (size_t i = 0; i < lIKDataCount; ++i)
        {
            IKData lIKData;

            // Read IK bone name
            lIKData.ikBoneName = internal::readStringField(
                pStream, internal::VMD_IK_BONE_NAME_FIELD_SIZE, "IK bone name");

            // Read IK state flag
            uint8_t lIkEnableFlag;
            internal::readFlagValue(pStream, lIkEnableFlag);
            if (lIkEnableFlag == 0)
                lIKData.ikState = IKState::OFF;
            else if (lIkEnableFlag == 1)
                lIKData.ikState = IKState::ON;
            else
            {
                throw exceptions::InvalidFieldValueError(
                    "IK state flag in the visible IK frame must be either 0 (OFF) or 1 (ON)."
                    "Frame number: " +
                    std::to_string(pVisibleIKFrame.frameNumber) +
                    " Invalid IK state flag: " + std::to_string(lIkEnableFlag));
            }

            pVisibleIKFrame.ikDataList.push_back(std::move(lIKData));
        }
    }

    void writeVisibleIKFrame(std::ofstream &pStream, const VisibleIKFrame &pVisibleIKFrame)
    {
        // Write frame number
        internal::writeUintValue(pStream, pVisibleIKFrame.frameNumber);

        // Write visible flag
        uint8_t lVisiblity = static_cast<uint8_t>(pVisibleIKFrame.visibility);
        internal::writeFlagValue(pStream, lVisiblity);

        // Write IK data count
        uint32_t lIKDataCount = static_cast<uint32_t>(pVisibleIKFrame.ikDataList.size());
        internal::writeUintValue(pStream, lIKDataCount);

        // Write IK data list
        for (const IKData &lIKData : pVisibleIKFrame.ikDataList)
        {
            // Write IK bone name
            internal::writeStringField(
                pStream, lIKData.ikBoneName, internal::VMD_IK_BONE_NAME_FIELD_SIZE, "IK bone name");

            // Write IK state flag
            uint8_t lIKState = static_cast<uint8_t>(lIKData.ikState);
            internal::writeFlagValue(pStream, lIKState);
        }
    }

    VMDData readVMD(const std::filesystem::path &pFilePath)
    {
        if (!std::filesystem::exists(pFilePath))
            throw exceptions::FileSystemError("File does not exist: " + pFilePath.string());

        if (pFilePath.extension() != ".vmd")
        {
            throw exceptions::IncompatibleFormatError(
                "File extension is not .vmd: " + pFilePath.string());
        }

        std::ifstream lFile(pFilePath, std::ios::binary);
        lFile.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);

        VMDData lVmdData;

        try
        {
            if (!lFile.is_open())
                throw exceptions::FileSystemError("Failed to open file: " + pFilePath.string());

            // Read header
            std::string lHeaderStr = internal::readHeader(lFile);

            // Validate header
            if (lHeaderStr != internal::VMD_HEADER)
            {
                throw exceptions::IncompatibleFormatError(
                    "Invalid VMD header. This file may not be a valid VMD file. Header read: " + lHeaderStr);
            }

            // Read model name
            std::string lModelName = internal::readStringField(
                lFile, internal::VMD_MODEL_NAME_HEADER_SIZE, "Model name");

            // Validate model name
            if (lModelName == internal::VMD_MODEL_NAME_CAMERA_EDIT)
            {
                throw exceptions::IncompatibleFormatError(
                    "This VMD file is for camera edit, not model edit. Model name: " + lModelName);
            }

            lVmdData.modelName = lModelName;

            uint64_t lTotalFrameCount = 0;

            // Read motion frame count and validate the total frame count
            uint32_t lMotionFrameCount;
            internal::readUintValue(lFile, lMotionFrameCount);
            internal::addAndValidateFrameCount(lTotalFrameCount, lMotionFrameCount);

            // Read motion frames
            lVmdData.motionFrames.reserve(lMotionFrameCount);
            for (size_t i = 0; i < lMotionFrameCount; ++i)
            {
                MotionFrame lMotionFrame;
                readMotionFrame(lFile, lMotionFrame);
                lVmdData.motionFrames.push_back(std::move(lMotionFrame));
            }

            // Read morph frame count and validate the total frame count
            uint32_t lMorphFrameCount;
            internal::readUintValue(lFile, lMorphFrameCount);
            internal::addAndValidateFrameCount(lTotalFrameCount, lMorphFrameCount);

            // Read morph frames
            lVmdData.morphFrames.reserve(lMorphFrameCount);
            for (size_t i = 0; i < lMorphFrameCount; ++i)
            {
                MorphFrame lMorphFrame;
                readMorphFrame(lFile, lMorphFrame);
                lVmdData.morphFrames.push_back(std::move(lMorphFrame));
            }

            // Read camera frame count
            uint32_t lCameraFrameCount;
            internal::readUintValue(lFile, lCameraFrameCount);

            // Fallback if the file somehow contains camera frames
            for (size_t i = 0; i < lCameraFrameCount; ++i)
            {
                lFile.ignore(internal::VMD_CAMERA_FRAME_SIZE);
            }

            // Read light frame count
            uint32_t lLightFrameCount;
            internal::readUintValue(lFile, lLightFrameCount);

            // Fallback if the file somehow contains light frames
            for (size_t i = 0; i < lLightFrameCount; ++i)
            {
                lFile.ignore(internal::VMD_LIGHT_FRAME_SIZE);
            }

            // Read self shadow frame count
            uint32_t lSelfShadowFrameCount;
            internal::readUintValue(lFile, lSelfShadowFrameCount);

            // Fallback if the file somehow contains self shadow frames
            for (size_t i = 0; i < lSelfShadowFrameCount; ++i)
            {
                lFile.ignore(internal::VMD_SELF_SHADOW_FRAME_SIZE);
            }

            // Read visible IK frame count and validate the total frame count
            uint32_t lVisibleIKFrameCount;
            internal::readUintValue(lFile, lVisibleIKFrameCount);
            internal::addAndValidateFrameCount(lTotalFrameCount, lVisibleIKFrameCount);

            // Read visible IK frames
            lVmdData.visibleIKFrames.reserve(lVisibleIKFrameCount);
            for (size_t i = 0; i < lVisibleIKFrameCount; ++i)
            {
                VisibleIKFrame lVisibleIKFrame;
                readVisibleIKFrame(lFile, lVisibleIKFrame);
                lVmdData.visibleIKFrames.push_back(std::move(lVisibleIKFrame));
            }
        }
        catch (const std::ios_base::failure &e)
        {
            if (lFile.eof())
            {
                throw exceptions::IncompatibleFormatError(
                    "Unexpected end of file reached while reading: " + pFilePath.string() +
                    ". The file may be corrupted or not a valid VMD file. Error: " +
                    e.what());
            }

            throw exceptions::FileSystemError(
                "An error occurred while reading the file: " + pFilePath.string() +
                " Error: " + e.what());
        }

        return lVmdData;
    }

    void writeVMD(const VMDData &pVmdData, const std::filesystem::path &pFilePath)
    {
        if (pFilePath.extension() != ".vmd")
        {
            throw exceptions::IncompatibleFormatError(
                "File extension is not .vmd: " + pFilePath.string());
        }

        // Validate data before writing
        validateModelName(pVmdData);
        validateFrameCount(pVmdData);
        validateMotionFrames(pVmdData);
        validateMorphFrames(pVmdData);
        validateVisibleIKFrames(pVmdData);

        std::ofstream lFile(pFilePath, std::ios::binary);
        lFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);

        try
        {
            if (!lFile.is_open())
            {
                throw exceptions::FileSystemError(
                    "Failed to open file for writing: " + pFilePath.string());
            }

            // Write header
            internal::writeHeader(lFile);

            // Write model name
            internal::writeStringField(
                lFile, pVmdData.modelName, internal::VMD_MODEL_NAME_HEADER_SIZE, "Model name");

            // Write motion frame count
            uint32_t lMotionFrameCount = static_cast<uint32_t>(pVmdData.motionFrames.size());
            internal::writeUintValue(lFile, lMotionFrameCount);

            // Write motion frames
            for (const MotionFrame &lMotionFrame : pVmdData.motionFrames)
            {
                writeMotionFrame(lFile, lMotionFrame);
            }

            // Write morph frame count
            uint32_t lMorphFrameCount = static_cast<uint32_t>(pVmdData.morphFrames.size());
            internal::writeUintValue(lFile, lMorphFrameCount);

            // Write morph frames
            for (const MorphFrame &lMorphFrame : pVmdData.morphFrames)
            {
                writeMorphFrame(lFile, lMorphFrame);
            }

            // Write camera, light, and self shadow frame counts as 0
            internal::writeUintValue(lFile, 0);
            internal::writeUintValue(lFile, 0);
            internal::writeUintValue(lFile, 0);

            // Write visible IK frame count
            uint32_t lVisibleIKFrameCount = static_cast<uint32_t>(pVmdData.visibleIKFrames.size());
            internal::writeUintValue(lFile, lVisibleIKFrameCount);

            // Write visible IK frames
            for (const VisibleIKFrame &lVisibleIKFrame : pVmdData.visibleIKFrames)
            {
                writeVisibleIKFrame(lFile, lVisibleIKFrame);
            }
        }
        catch (const std::ios_base::failure &e)
        {
            if (lFile.eof())
            {
                throw exceptions::IncompatibleFormatError(
                    "Unexpected end of file reached while writing: " + pFilePath.string() +
                    ". The file may be corrupted or not a valid VMD file. Error: " +
                    e.what());
            }

            throw exceptions::FileSystemError(
                "An error occurred while writing to the file: " + pFilePath.string() +
                " Error: " + e.what());
        }
    }
}