#include "camera_edit.h"

#include <string>
#include <string_view>
#include <unordered_set>
#define _USE_MATH_DEFINES
#include <cmath>

#include "encoding.h"
#include "exceptions.h"
#include "private/vmd_constants.h"
#include "private/vmd_io_utils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_1_PI
#define M_1_PI 0.318309886183790671538
#endif

namespace
{
    inline void validateFrameCount(const vmdio::camera_edit::VMDData &pVmdData)
    {
        size_t lTotalFrames = pVmdData.cameraFrames.size() + pVmdData.lightFrames.size() +
                              pVmdData.selfShadowFrames.size();

        if (lTotalFrames > vmdio::internal::MAX_FRAME_COUNT)
        {
            throw vmdio::exceptions::FrameOverflowError(
                "Total frame count in the VMD data exceeds the supported maximum limit of " + std::to_string(vmdio::internal::MAX_FRAME_COUNT) +
                ". Total frames: " + std::to_string(lTotalFrames));
        }
    }

    inline void validateCameraInterpolation(
        const vmdio::camera_edit::CameraInterpolation &pInterpolation, uint32_t pFrameNumber)
    {
        vmdio::internal::validateControlPointSet(
            pInterpolation.xPos, "CameraInterpolation.xPos", pFrameNumber);
        vmdio::internal::validateControlPointSet(
            pInterpolation.yPos, "CameraInterpolation.yPos", pFrameNumber);
        vmdio::internal::validateControlPointSet(
            pInterpolation.zPos, "CameraInterpolation.zPos", pFrameNumber);
        vmdio::internal::validateControlPointSet(
            pInterpolation.rot, "CameraInterpolation.rot", pFrameNumber);
        vmdio::internal::validateControlPointSet(
            pInterpolation.dist, "CameraInterpolation.dist", pFrameNumber);
        vmdio::internal::validateControlPointSet(
            pInterpolation.view, "CameraInterpolation.view", pFrameNumber);
    }

    inline void validateCameraFrames(const vmdio::camera_edit::VMDData &pVmdData)
    {
        std::unordered_set<uint32_t> lFrameNumberSet;

        for (const auto &lCameraFrame : pVmdData.cameraFrames)
        {
            auto [it, lInserted] = lFrameNumberSet.insert(lCameraFrame.frameNumber);

            if (!lInserted)
            {
                throw vmdio::exceptions::FrameConflictError(
                    "Duplicate camera frame detected with frame number " +
                    std::to_string(lCameraFrame.frameNumber));
            }

            validateCameraInterpolation(lCameraFrame.interpolation, lCameraFrame.frameNumber);

            if (lCameraFrame.projectionType != vmdio::camera_edit::ProjectionType::Perspective &&
                lCameraFrame.projectionType != vmdio::camera_edit::ProjectionType::Orthographic)
            {
                throw vmdio::exceptions::InvalidFieldValueError(
                    "Projection type in the camera frame must be either Perspective (0) or Orthographic (1). Frame number: " + std::to_string(lCameraFrame.frameNumber) +
                    " Invalid projection type: " +
                    std::to_string(static_cast<uint32_t>(lCameraFrame.projectionType)));
            }
        }
    }

    inline void validateLightFrames(const vmdio::camera_edit::VMDData &pVmdData)
    {
        std::unordered_set<uint32_t> lFrameNumberSet;

        for (const auto &lLightFrame : pVmdData.lightFrames)
        {
            auto [it, lInserted] = lFrameNumberSet.insert(lLightFrame.frameNumber);

            if (!lInserted)
            {
                throw vmdio::exceptions::FrameConflictError(
                    "Duplicate light frame detected with frame number " +
                    std::to_string(lLightFrame.frameNumber));
            }
        }
    }

    inline void validateSelfShadowFrames(const vmdio::camera_edit::VMDData &pVmdData)
    {
        std::unordered_set<uint32_t> lFrameNumberSet;

        for (const auto &lSelfShadowFrame : pVmdData.selfShadowFrames)
        {
            auto [it, lInserted] = lFrameNumberSet.insert(lSelfShadowFrame.frameNumber);

            if (!lInserted)
            {
                throw vmdio::exceptions::FrameConflictError(
                    "Duplicate self shadow frame detected with frame number " +
                    std::to_string(lSelfShadowFrame.frameNumber));
            }

            if (lSelfShadowFrame.mode != vmdio::camera_edit::SelfShadowMode::NoSelfShadow &&
                lSelfShadowFrame.mode != vmdio::camera_edit::SelfShadowMode::Mode1 &&
                lSelfShadowFrame.mode != vmdio::camera_edit::SelfShadowMode::Mode2)
            {
                throw vmdio::exceptions::InvalidFieldValueError(
                    "Self shadow mode in the self shadow frame must be either NoSelfShadow (0), Mode1 (1), or Mode2 (2). Frame number: " +
                    std::to_string(lSelfShadowFrame.frameNumber) +
                    " Invalid self shadow mode: " +
                    std::to_string(static_cast<uint32_t>(lSelfShadowFrame.mode)));
            }
        }
    }
}

namespace vmdio::camera_edit
{
    void readCameraInterpolation(std::ifstream &pStream, CameraInterpolation &pInterpolation)
    {
        // Read interpolation
        uint8_t lInterpBuffer[internal::VMD_CAMERA_INTERPOLATION_FIELD_SIZE];
        internal::readInterpolationBuffer(pStream, lInterpBuffer, sizeof(lInterpBuffer));

        pInterpolation.xPos.x1 = static_cast<uint32_t>(lInterpBuffer[0]);
        pInterpolation.xPos.y1 = static_cast<uint32_t>(lInterpBuffer[2]);
        pInterpolation.xPos.x2 = static_cast<uint32_t>(lInterpBuffer[1]);
        pInterpolation.xPos.y2 = static_cast<uint32_t>(lInterpBuffer[3]);

        pInterpolation.yPos.x1 = static_cast<uint32_t>(lInterpBuffer[4]);
        pInterpolation.yPos.y1 = static_cast<uint32_t>(lInterpBuffer[6]);
        pInterpolation.yPos.x2 = static_cast<uint32_t>(lInterpBuffer[5]);
        pInterpolation.yPos.y2 = static_cast<uint32_t>(lInterpBuffer[7]);

        pInterpolation.zPos.x1 = static_cast<uint32_t>(lInterpBuffer[8]);
        pInterpolation.zPos.y1 = static_cast<uint32_t>(lInterpBuffer[10]);
        pInterpolation.zPos.x2 = static_cast<uint32_t>(lInterpBuffer[9]);
        pInterpolation.zPos.y2 = static_cast<uint32_t>(lInterpBuffer[11]);

        pInterpolation.rot.x1 = static_cast<uint32_t>(lInterpBuffer[12]);
        pInterpolation.rot.y1 = static_cast<uint32_t>(lInterpBuffer[14]);
        pInterpolation.rot.x2 = static_cast<uint32_t>(lInterpBuffer[13]);
        pInterpolation.rot.y2 = static_cast<uint32_t>(lInterpBuffer[15]);

        pInterpolation.dist.x1 = static_cast<uint32_t>(lInterpBuffer[16]);
        pInterpolation.dist.y1 = static_cast<uint32_t>(lInterpBuffer[18]);
        pInterpolation.dist.x2 = static_cast<uint32_t>(lInterpBuffer[17]);
        pInterpolation.dist.y2 = static_cast<uint32_t>(lInterpBuffer[19]);

        pInterpolation.view.x1 = static_cast<uint32_t>(lInterpBuffer[20]);
        pInterpolation.view.y1 = static_cast<uint32_t>(lInterpBuffer[22]);
        pInterpolation.view.x2 = static_cast<uint32_t>(lInterpBuffer[21]);
        pInterpolation.view.y2 = static_cast<uint32_t>(lInterpBuffer[23]);
    }

    void writeCameraInterpolation(std::ofstream &pStream, const CameraInterpolation &pInterpolation)
    {
        // Initialize buffer with zeros
        uint8_t lInterpBuffer[internal::VMD_CAMERA_INTERPOLATION_FIELD_SIZE] = {};

        lInterpBuffer[0] = static_cast<uint8_t>(pInterpolation.xPos.x1);
        lInterpBuffer[2] = static_cast<uint8_t>(pInterpolation.xPos.y1);
        lInterpBuffer[1] = static_cast<uint8_t>(pInterpolation.xPos.x2);
        lInterpBuffer[3] = static_cast<uint8_t>(pInterpolation.xPos.y2);

        lInterpBuffer[4] = static_cast<uint8_t>(pInterpolation.yPos.x1);
        lInterpBuffer[6] = static_cast<uint8_t>(pInterpolation.yPos.y1);
        lInterpBuffer[5] = static_cast<uint8_t>(pInterpolation.yPos.x2);
        lInterpBuffer[7] = static_cast<uint8_t>(pInterpolation.yPos.y2);

        lInterpBuffer[8] = static_cast<uint8_t>(pInterpolation.zPos.x1);
        lInterpBuffer[10] = static_cast<uint8_t>(pInterpolation.zPos.y1);
        lInterpBuffer[9] = static_cast<uint8_t>(pInterpolation.zPos.x2);
        lInterpBuffer[11] = static_cast<uint8_t>(pInterpolation.zPos.y2);

        lInterpBuffer[12] = static_cast<uint8_t>(pInterpolation.rot.x1);
        lInterpBuffer[14] = static_cast<uint8_t>(pInterpolation.rot.y1);
        lInterpBuffer[13] = static_cast<uint8_t>(pInterpolation.rot.x2);
        lInterpBuffer[15] = static_cast<uint8_t>(pInterpolation.rot.y2);

        lInterpBuffer[16] = static_cast<uint8_t>(pInterpolation.dist.x1);
        lInterpBuffer[18] = static_cast<uint8_t>(pInterpolation.dist.y1);
        lInterpBuffer[17] = static_cast<uint8_t>(pInterpolation.dist.x2);
        lInterpBuffer[19] = static_cast<uint8_t>(pInterpolation.dist.y2);

        lInterpBuffer[20] = static_cast<uint8_t>(pInterpolation.view.x1);
        lInterpBuffer[22] = static_cast<uint8_t>(pInterpolation.view.y1);
        lInterpBuffer[21] = static_cast<uint8_t>(pInterpolation.view.x2);
        lInterpBuffer[23] = static_cast<uint8_t>(pInterpolation.view.y2);

        internal::writeInterpolationBuffer(pStream, lInterpBuffer, sizeof(lInterpBuffer));
    }

    void readCameraFrame(std::ifstream &pStream, CameraFrame &pCameraFrame)
    {
        // Read frame number
        internal::readUintValue(pStream, pCameraFrame.frameNumber);

        // Read distance
        float lDistanceRaw;
        internal::readFloatValue(pStream, lDistanceRaw);
        pCameraFrame.distance = lDistanceRaw * (-1.0f); // Invert sign to convert to UI value

        // Read position
        internal::readFloatValue(pStream, pCameraFrame.position.x);
        internal::readFloatValue(pStream, pCameraFrame.position.y);
        internal::readFloatValue(pStream, pCameraFrame.position.z);

        // Read rotation
        float lXRad, lYRad, lZRad;
        internal::readFloatValue(pStream, lXRad);
        internal::readFloatValue(pStream, lYRad);
        internal::readFloatValue(pStream, lZRad);

        // Convert radians to degrees and invert x rotation sign to convert to UI values
        pCameraFrame.rotation.eulerX = lXRad * M_1_PI * 180.0f * (-1.0f);
        pCameraFrame.rotation.eulerY = lYRad * M_1_PI * 180.0f;
        pCameraFrame.rotation.eulerZ = lZRad * M_1_PI * 180.0f;

        // Read interpolation
        readCameraInterpolation(pStream, pCameraFrame.interpolation);

        // Read viewing angle
        internal::readIntValue(pStream, pCameraFrame.viewingAngle);

        // Read projection type
        uint8_t lProjectionType;
        internal::readFlagValue(pStream, lProjectionType);

        if (lProjectionType == 0)
            pCameraFrame.projectionType = ProjectionType::Perspective;
        else if (lProjectionType == 1)
            pCameraFrame.projectionType = ProjectionType::Orthographic;
        else
        {
            throw exceptions::InvalidFieldValueError(
                "Projection type in the camera frame must be either Perspective (0) or Orthographic (1). Frame number: " + std::to_string(pCameraFrame.frameNumber) +
                " Invalid projection type: " + std::to_string(lProjectionType));
        }
    }

    void writeCameraFrame(std::ofstream &pStream, const CameraFrame &pCameraFrame)
    {
        // Write frame number
        internal::writeUintValue(pStream, pCameraFrame.frameNumber);

        // Write distance with sign inverted
        float lDistanceInverted = pCameraFrame.distance * (-1.0f);
        internal::writeFloatValue(pStream, lDistanceInverted);

        // Write position
        internal::writeFloatValue(pStream, pCameraFrame.position.x);
        internal::writeFloatValue(pStream, pCameraFrame.position.y);
        internal::writeFloatValue(pStream, pCameraFrame.position.z);

        // Write rotation with conversion from degrees to radians and x rotation sign inverted
        float lXRadInverted = pCameraFrame.rotation.eulerX * M_PI / 180.0f * (-1.0f);
        float lYRad = pCameraFrame.rotation.eulerY * M_PI / 180.0f;
        float lZRad = pCameraFrame.rotation.eulerZ * M_PI / 180.0f;
        internal::writeFloatValue(pStream, lXRadInverted);
        internal::writeFloatValue(pStream, lYRad);
        internal::writeFloatValue(pStream, lZRad);

        // Write interpolation
        writeCameraInterpolation(pStream, pCameraFrame.interpolation);

        // Write viewing angle
        internal::writeIntValue(pStream, pCameraFrame.viewingAngle);

        // Write projection type
        uint8_t lProjectionType = static_cast<uint8_t>(pCameraFrame.projectionType);
        internal::writeFlagValue(pStream, lProjectionType);
    }

    void readLightFrame(std::ifstream &pStream, LightFrame &pLightFrame)
    {
        // Read frame number
        internal::readUintValue(pStream, pLightFrame.frameNumber);

        // Read color
        float lR, lG, lB;
        internal::readFloatValue(pStream, lR);
        internal::readFloatValue(pStream, lG);
        internal::readFloatValue(pStream, lB);

        // Convert back to UI values
        pLightFrame.color.r = static_cast<int32_t>(std::round(lR * 256.0f));
        pLightFrame.color.g = static_cast<int32_t>(std::round(lG * 256.0f));
        pLightFrame.color.b = static_cast<int32_t>(std::round(lB * 256.0f));

        // Read position
        internal::readFloatValue(pStream, pLightFrame.position.x);
        internal::readFloatValue(pStream, pLightFrame.position.y);
        internal::readFloatValue(pStream, pLightFrame.position.z);
    }

    void writeLightFrame(std::ofstream &pStream, const LightFrame &pLightFrame)
    {
        // Write frame number
        internal::writeUintValue(pStream, pLightFrame.frameNumber);

        // Write color with conversion to VMD file value
        float lR = pLightFrame.color.r / 256.0f;
        float lG = pLightFrame.color.g / 256.0f;
        float lB = pLightFrame.color.b / 256.0f;
        internal::writeFloatValue(pStream, lR);
        internal::writeFloatValue(pStream, lG);
        internal::writeFloatValue(pStream, lB);

        // Write position
        internal::writeFloatValue(pStream, pLightFrame.position.x);
        internal::writeFloatValue(pStream, pLightFrame.position.y);
        internal::writeFloatValue(pStream, pLightFrame.position.z);
    }

    void readSelfShadowFrame(std::ifstream &pStream, SelfShadowFrame &pSelfShadowFrame)
    {
        // Read frame number
        internal::readUintValue(pStream, pSelfShadowFrame.frameNumber);

        // Read mode
        uint8_t lMode;
        internal::readFlagValue(pStream, lMode);

        if (lMode == 0)
            pSelfShadowFrame.mode = SelfShadowMode::NoSelfShadow;
        else if (lMode == 1)
            pSelfShadowFrame.mode = SelfShadowMode::Mode1;
        else if (lMode == 2)
            pSelfShadowFrame.mode = SelfShadowMode::Mode2;
        else
        {
            throw exceptions::InvalidFieldValueError(
                "Self shadow mode in the self shadow frame must be either NoSelfShadow (0), Mode1 (1), or Mode2 (2). Frame number: " + std::to_string(pSelfShadowFrame.frameNumber) +
                " Invalid self shadow mode: " + std::to_string(lMode));
        }

        // Read distance (float)
        float lDistanceRaw;
        internal::readFloatValue(pStream, lDistanceRaw);

        // Convert to UI value
        int32_t lDistance = static_cast<int32_t>(std::round((0.1f - lDistanceRaw) * 100000.0f));
        pSelfShadowFrame.shadowRange = lDistance;
    }

    void writeSelfShadowFrame(std::ofstream &pStream, const SelfShadowFrame &pSelfShadowFrame)
    {
        // Write frame number
        internal::writeUintValue(pStream, pSelfShadowFrame.frameNumber);

        // Write mode
        uint8_t lMode = static_cast<uint8_t>(pSelfShadowFrame.mode);
        internal::writeFlagValue(pStream, lMode);

        // Write distance with conversion to VMD file value
        float lDistance = 0.1f - (pSelfShadowFrame.shadowRange * 0.00001f);
        internal::writeFloatValue(pStream, lDistance);
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
                lFile, internal::VMD_MODEL_NAME_HEADER_SIZE, "model name");

            // Validate model name
            if (lModelName != internal::VMD_MODEL_NAME_CAMERA_EDIT)
            {
                throw exceptions::IncompatibleFormatError(
                    "This VMD file is for model edit, not camera edit. Model name: " + lModelName);
            }

            uint64_t lTotalFrameCount = 0;

            // Read motion frame count
            uint32_t lMotionFrameCount;
            internal::readUintValue(lFile, lMotionFrameCount);

            // Fallback if the file somehow contains motion frames
            for (size_t i = 0; i < lMotionFrameCount; ++i)
            {
                lFile.ignore(internal::VMD_MOTION_FRAME_SIZE);
            }

            // Read morph frame count
            uint32_t lMorphFrameCount;
            internal::readUintValue(lFile, lMorphFrameCount);

            // Fallback if the file somehow contains morph frames
            for (size_t i = 0; i < lMorphFrameCount; ++i)
            {
                lFile.ignore(internal::VMD_MORPH_FRAME_SIZE);
            }

            // Read camera frame count and validate the total frame count
            uint32_t lCameraFrameCount;
            internal::readUintValue(lFile, lCameraFrameCount);
            internal::addAndValidateFrameCount(lTotalFrameCount, lCameraFrameCount);

            // Read camera frames
            lVmdData.cameraFrames.reserve(lCameraFrameCount);
            for (size_t i = 0; i < lCameraFrameCount; ++i)
            {
                CameraFrame lCameraFrame;
                readCameraFrame(lFile, lCameraFrame);
                lVmdData.cameraFrames.push_back(std::move(lCameraFrame));
            }

            // Read light frame count and validate the total frame count
            uint32_t lLightFrameCount;
            internal::readUintValue(lFile, lLightFrameCount);
            internal::addAndValidateFrameCount(lTotalFrameCount, lLightFrameCount);

            // Read light frames
            lVmdData.lightFrames.reserve(lLightFrameCount);
            for (size_t i = 0; i < lLightFrameCount; ++i)
            {
                LightFrame lLightFrame;
                readLightFrame(lFile, lLightFrame);
                lVmdData.lightFrames.push_back(std::move(lLightFrame));
            }

            // Read self shadow frame count and validate the total frame count
            uint32_t lSelfShadowFrameCount;
            internal::readUintValue(lFile, lSelfShadowFrameCount);
            internal::addAndValidateFrameCount(lTotalFrameCount, lSelfShadowFrameCount);

            // Read self shadow frames
            lVmdData.selfShadowFrames.reserve(lSelfShadowFrameCount);
            for (size_t i = 0; i < lSelfShadowFrameCount; ++i)
            {
                SelfShadowFrame lSelfShadowFrame;
                readSelfShadowFrame(lFile, lSelfShadowFrame);
                lVmdData.selfShadowFrames.push_back(std::move(lSelfShadowFrame));
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
        validateFrameCount(pVmdData);
        validateCameraFrames(pVmdData);
        validateLightFrames(pVmdData);
        validateSelfShadowFrames(pVmdData);

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
                lFile, internal::VMD_MODEL_NAME_CAMERA_EDIT,
                internal::VMD_MODEL_NAME_HEADER_SIZE, "Model name");

            // Write motion and morph frame counts as 0
            internal::writeUintValue(lFile, 0);
            internal::writeUintValue(lFile, 0);

            // Write camera frame count
            uint32_t lCameraFrameCount = static_cast<uint32_t>(pVmdData.cameraFrames.size());
            internal::writeUintValue(lFile, lCameraFrameCount);

            // Write camera frames
            for (const CameraFrame &lCameraFrame : pVmdData.cameraFrames)
            {
                writeCameraFrame(lFile, lCameraFrame);
            }

            // Write light frame count
            uint32_t lLightFrameCount = static_cast<uint32_t>(pVmdData.lightFrames.size());
            internal::writeUintValue(lFile, lLightFrameCount);

            // Write light frames
            for (const LightFrame &lLightFrame : pVmdData.lightFrames)
            {
                writeLightFrame(lFile, lLightFrame);
            }

            // Write self shadow frame count
            uint32_t lSelfShadowFrameCount =
                static_cast<uint32_t>(pVmdData.selfShadowFrames.size());
            internal::writeUintValue(lFile, lSelfShadowFrameCount);

            // Write self shadow frames
            for (const SelfShadowFrame &lSelfShadowFrame : pVmdData.selfShadowFrames)
            {
                writeSelfShadowFrame(lFile, lSelfShadowFrame);
            }

            // Write visible IK frame count as 0
            internal::writeUintValue(lFile, 0);
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