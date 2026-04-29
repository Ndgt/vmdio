// ModelMotionImport.cpp
// usage:
//   ModelMotionImport <path to vmd file> > output.txt

#include <vmdio/model_edit.h>
#include <vmdio/vmd_exceptions.h>

#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace vmd = vmdio::model_edit;
namespace vmd_except = vmdio::exceptions;

void printSummaryData(const vmd::VMDData &pVmdData)
{
    std::cout << "\n=== Summary of imported data ===\n";
    std::cout << "Model Name: " << pVmdData.modelName.toUTF8ForDisplay() << "\n";
    std::cout << "Motion Frame Count: " << pVmdData.motionFrames.size() << "\n";
    std::cout << "Morph Frame Count: " << pVmdData.morphFrames.size() << "\n";
    std::cout << "Visible IK Frame Count: " << pVmdData.visibleIKFrames.size() << "\n";
}

void printMotionFrames(const vmd::VMDData &pVmdData)
{
    if (pVmdData.motionFrames.empty())
        return;

    std::cout << "\n=== Motion Frames ===" << "\n";

    for (const auto &lMotionFrame : pVmdData.motionFrames)
    {
        vmd::Position lPos = lMotionFrame.position;
        vmd::Quaternion lRot = lMotionFrame.rotation;
        vmd::MotionInterpolation lInterp = lMotionFrame.interpolation;

        std::cout << "Motion Frame: " << lMotionFrame.frameNumber << "\n";
        std::cout << "  Bone Name: " << lMotionFrame.boneName.toUTF8ForDisplay() << "\n";
        std::cout << "  Position: (" << lPos.x << ", " << lPos.y << ", " << lPos.z << ")\n";
        std::cout << "  Rotation: (" << lRot.qx << ", " << lRot.qy << ", " << lRot.qz << ", " << lRot.qw << ")\n";
        std::cout << "  Interpolation: " << "\n";
        std::cout << "    X Position: (" << lInterp.xPos.x1 << ", " << lInterp.xPos.y1 << ", " << lInterp.xPos.x2 << ", " << lInterp.xPos.y2 << ")\n";
        std::cout << "    Y Position: (" << lInterp.yPos.x1 << ", " << lInterp.yPos.y1 << ", " << lInterp.yPos.x2 << ", " << lInterp.yPos.y2 << ")\n";
        std::cout << "    Z Position: (" << lInterp.zPos.x1 << ", " << lInterp.zPos.y1 << ", " << lInterp.zPos.x2 << ", " << lInterp.zPos.y2 << ")\n";
        std::cout << "    Rotation:   (" << lInterp.rot.x1 << ", " << lInterp.rot.y1 << ", " << lInterp.rot.x2 << ", " << lInterp.rot.y2 << ")\n";
        std::cout << "\n";
    }
}

void printMorphFrames(const vmd::VMDData &pVmdData)
{
    if (pVmdData.morphFrames.empty())
        return;

    std::cout << "\n=== Morph Frames ===\n";

    for (const auto &lMorphFrame : pVmdData.morphFrames)
    {
        std::cout << "Morph Frame: " << lMorphFrame.frameNumber << "\n";
        std::cout << "  Morph Name: " << lMorphFrame.morphName.toUTF8ForDisplay() << "\n";
        std::cout << "  Value: " << lMorphFrame.value << "\n";
        std::cout << "\n";
    }
}

void printVisibleIKFrames(const vmd::VMDData &pVmdData)
{
    if (pVmdData.visibleIKFrames.empty())
        return;

    std::cout << "\n=== Visible IK Frames ===\n";

    for (const auto &lVisibleIKFrame : pVmdData.visibleIKFrames)
    {
        std::string lVisibilityStr = "Unknown";
        switch (lVisibleIKFrame.visibility)
        {
        case vmd::Visibility::Hidden:
            lVisibilityStr = "Hidden";
            break;
        case vmd::Visibility::Visible:
            lVisibilityStr = "Visible";
            break;
        default:
            break;
        }

        std::cout << "Visible IK Frame: " << lVisibleIKFrame.frameNumber << "\n";
        std::cout << "  Visibility: " << lVisibilityStr << "\n";

        if (lVisibleIKFrame.ikDataList.empty())
            continue;

        std::cout << "  IK Data:\n";

        for (const auto &lIKData : lVisibleIKFrame.ikDataList)
        {
            std::string lIKStateStr = "Unknown";
            switch (lIKData.ikState)
            {
            case vmd::IKState::OFF:
                lIKStateStr = "OFF";
                break;
            case vmd::IKState::ON:
                lIKStateStr = "ON";
                break;
            default:
                break;
            }

            std::cout << "    IK Bone Name: " << lIKData.ikBoneName.toUTF8ForDisplay()
                      << ", IK State: " << lIKStateStr << "\n";
        }

        std::cout << "\n";
    }
}

int main(int argc, char *argv[])
{
    try
    {
        if (argc < 2)
            throw std::runtime_error("No VMD file path provided.");

        std::filesystem::path lVMDFilePath = argv[1];
        if (!std::filesystem::exists(lVMDFilePath))
            throw std::runtime_error("File does not exist: " + lVMDFilePath.string());

        // Read VMD file and populate data structure
        vmd::VMDData lVmdDataForModelEdit = vmd::readVMD(lVMDFilePath);

        // Print the summary of the imported data
        printSummaryData(lVmdDataForModelEdit);

        // Print all frames for each type of data
        printMotionFrames(lVmdDataForModelEdit);
        printMorphFrames(lVmdDataForModelEdit);
        printVisibleIKFrames(lVmdDataForModelEdit);
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