// CameraMotionImport.cpp
// usage:
//   CameraMotionImport <path to vmd file> > output.txt

#include <vmdio/camera_edit.h>
#include <vmdio/vmd_exceptions.h>

#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace vmd = vmdio::camera_edit;
namespace vmd_except = vmdio::exceptions;

void printSummaryData(const vmd::VMDData &pVmdData)
{
    std::cout << "\n=== Summary of imported data ===\n";
    std::cout << "Camera Frame Count: " << pVmdData.cameraFrames.size() << "\n";
    std::cout << "Light Frame Count: " << pVmdData.lightFrames.size() << "\n";
    std::cout << "Self Shadow Frame Count: " << pVmdData.selfShadowFrames.size() << "\n";
}

void printCameraFrames(const vmd::VMDData &pVmdData)
{
    if (pVmdData.cameraFrames.empty())
        return;

    std::cout << "\n=== Camera Frames ===\n";

    for (const auto &lCameraFrame : pVmdData.cameraFrames)
    {
        const vmd::Position lPos = lCameraFrame.position;
        const vmd::CameraRotation lRot = lCameraFrame.rotation;
        const vmd::CameraInterpolation lInterp = lCameraFrame.interpolation;
        std::string lProjectionTypeStr = "Unknown";

        switch (lCameraFrame.projectionType)
        {
        case vmd::ProjectionType::Perspective:
            lProjectionTypeStr = "Perspective";
            break;
        case vmd::ProjectionType::Orthographic:
            lProjectionTypeStr = "Orthographic";
            break;
        default:
            break;
        }

        std::cout << "Camera Frame: " << lCameraFrame.frameNumber << "\n";
        std::cout << "  Distance: " << lCameraFrame.distance << "\n";
        std::cout << "  Position: (" << lPos.x << ", " << lPos.y << ", " << lPos.z << ")\n";
        std::cout << "  Rotation: (" << lRot.eulerX << ", " << lRot.eulerY << ", " << lRot.eulerZ << ")\n";
        std::cout << "  Viewing Angle: " << lCameraFrame.viewingAngle << "\n";
        std::cout << "  Projection Type: " << lProjectionTypeStr << "\n";
        std::cout << "  Interpolation:\n";
        std::cout << "    X Position:    (" << lInterp.xPos.x1 << ", " << lInterp.xPos.y1 << ", " << lInterp.xPos.x2 << ", " << lInterp.xPos.y2 << ")\n";
        std::cout << "    Y Position:    (" << lInterp.yPos.x1 << ", " << lInterp.yPos.y1 << ", " << lInterp.yPos.x2 << ", " << lInterp.yPos.y2 << ")\n";
        std::cout << "    Z Position:    (" << lInterp.zPos.x1 << ", " << lInterp.zPos.y1 << ", " << lInterp.zPos.x2 << ", " << lInterp.zPos.y2 << ")\n";
        std::cout << "    Rotation:      (" << lInterp.rot.x1 << ", " << lInterp.rot.y1 << ", " << lInterp.rot.x2 << ", " << lInterp.rot.y2 << ")\n";
        std::cout << "    Distance:      (" << lInterp.dist.x1 << ", " << lInterp.dist.y1 << ", " << lInterp.dist.x2 << ", " << lInterp.dist.y2 << ")\n";
        std::cout << "    Viewing Angle: (" << lInterp.view.x1 << ", " << lInterp.view.y1 << ", " << lInterp.view.x2 << ", " << lInterp.view.y2 << ")\n";
        std::cout << "\n";
    }
}

void printLightFrames(const vmd::VMDData &pVmdData)
{
    if (pVmdData.lightFrames.empty())
        return;

    std::cout << "\n=== Light Frames ===\n";

    for (const auto &lLightFrame : pVmdData.lightFrames)
    {
        const vmd::Color lColor = lLightFrame.color;
        const vmd::Position lPos = lLightFrame.position;

        std::cout << "Light Frame: " << lLightFrame.frameNumber << "\n";
        std::cout << "  Color: (" << lColor.r << ", " << lColor.g << ", " << lColor.b << ")\n";
        std::cout << "  Position: (" << lPos.x << ", " << lPos.y << ", " << lPos.z << ")\n";
        std::cout << "\n";
    }
}

void printSelfShadowFrames(const vmd::VMDData &pVmdData)
{
    if (pVmdData.selfShadowFrames.empty())
        return;

    std::cout << "\n=== Self Shadow Frames ===\n";

    for (const auto &lSelfShadowFrame : pVmdData.selfShadowFrames)
    {
        std::string lSelfShadowModeStr = "Unknown";

        switch (lSelfShadowFrame.mode)
        {
        case vmd::SelfShadowMode::NoSelfShadow:
            lSelfShadowModeStr = "No Self Shadow";
            break;
        case vmd::SelfShadowMode::Mode1:
            lSelfShadowModeStr = "Mode 1";
            break;
        case vmd::SelfShadowMode::Mode2:
            lSelfShadowModeStr = "Mode 2";
            break;
        default:
            break;
        }

        std::cout << "Self Shadow Frame: " << lSelfShadowFrame.frameNumber << "\n";
        std::cout << "  Shadow Range: " << lSelfShadowFrame.shadowRange << "\n";
        std::cout << "  Self Shadow Mode: " << lSelfShadowModeStr << "\n";
        std::cout << "\n";
    }
}

int main(int argc, char *argv[])
{
    try
    {
        if (argc < 2)
            throw std::runtime_error("No VMD file path provided.");

        const std::filesystem::path lVMDFilePath = argv[1];

        if (!std::filesystem::exists(lVMDFilePath))
            throw std::runtime_error("File does not exist: " + lVMDFilePath.string());

        // Read a camera edit VMD file and populate the camera_edit::VMDData structure
        const vmd::VMDData lVmdDataForCameraEdit = vmd::readVMD(lVMDFilePath);

        // Print the summary of imported data.
        printSummaryData(lVmdDataForCameraEdit);

        // Print all frames for each type of data.
        printCameraFrames(lVmdDataForCameraEdit);
        printLightFrames(lVmdDataForCameraEdit);
        printSelfShadowFrames(lVmdDataForCameraEdit);
    }

    // Catch vmdio exceptions
    catch (const vmd_except::VMDIOError &e)
    {
        std::cerr << "vmdio library threw an error: " << e.what() << "\n";
        return 1;
    }

    // Catch any other standard exceptions
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}