# camera_motion_import.py
# usage:
#    python camera_motion_import.py path_to_your_motion.vmd [-v] > output.txt

from pathlib import Path
import argparse
import sys
import pyvmdio.camera_edit as vmdio
import pyvmdio.exceptions as vmdio_exceptions


def print_summary_data(vmd_data: vmdio.VMDData) -> None:
    print("\n=== Summary of imported data ===")
    print(f"Camera Frame Count: {len(vmd_data.cameraFrames)}")
    print(f"Light Frame Count: {len(vmd_data.lightFrames)}")
    print(f"Self Shadow Frame Count: {len(vmd_data.selfShadowFrames)}")

    # Note:
    # Frame lists such as cameraFrames, lightFrames, and selfShadowFrames
    # are list-like wrappers around C++ std::vector.
    # You can iterate over them and append frames, but do not keep a frame object
    # obtained from a list while changing the same list with append/insert/remove/clear.
    # After changing the list, get the frame again from the list.

def print_camera_frames(vmd_data: vmdio.VMDData) -> None:
    if len(vmd_data.cameraFrames) == 0:
        return

    print("\n=== Camera Frames ===")

    for camera_frame in vmd_data.cameraFrames:
        pos = camera_frame.position
        rot = camera_frame.rotation
        interp = camera_frame.interpolation
        projection_type_str = "Unknown"

        match camera_frame.projectionType:
            case vmdio.ProjectionType.Perspective:
                projection_type_str = "Perspective"
            case vmdio.ProjectionType.Orthographic:
                projection_type_str = "Orthographic"

        print(f"Camera Frame: {camera_frame.frameNumber}")
        print(f"  Position: ({pos.x}, {pos.y}, {pos.z})")
        print(f"  Rotation: ({rot.eulerX}, {rot.eulerY}, {rot.eulerZ})")
        print(f"  Distance: {camera_frame.distance}")
        print(f"  Viewing Angle: {camera_frame.viewingAngle}")
        print(f"  Projection Type: {projection_type_str}")
        print("  Interpolation:")
        print(f"     X Position:    ({interp.xPos.x1}, {interp.xPos.y1}, {interp.xPos.x2}, {interp.xPos.y2})")
        print(f"     Y Position:    ({interp.yPos.x1}, {interp.yPos.y1}, {interp.yPos.x2}, {interp.yPos.y2})")
        print(f"     Z Position:    ({interp.zPos.x1}, {interp.zPos.y1}, {interp.zPos.x2}, {interp.zPos.y2})")
        print(f"     Rotation:      ({interp.rot.x1}, {interp.rot.y1}, {interp.rot.x2}, {interp.rot.y2})")
        print(f"     Distance:      ({interp.dist.x1}, {interp.dist.y1}, {interp.dist.x2}, {interp.dist.y2})")
        print(f"     Viewing Angle: ({interp.view.x1}, {interp.view.y1}, {interp.view.x2}, {interp.view.y2})")
        print()


def print_light_frames(vmd_data: vmdio.VMDData) -> None:
    if len(vmd_data.lightFrames) == 0:
        return

    print("\n=== Light Frames ===")

    for light_frame in vmd_data.lightFrames:
        print(f"Light Frame: {light_frame.frameNumber}")
        print(f"  Color: ({light_frame.color.r}, {light_frame.color.g}, {light_frame.color.b})")
        print(f"  Position: ({light_frame.position.x}, {light_frame.position.y}, {light_frame.position.z})")
        print()


def print_selfshadow_frames(vmd_data: vmdio.VMDData) -> None:
    if len(vmd_data.selfShadowFrames) == 0:
        return

    print("\n=== Self Shadow Frames ===")

    for shadow_frame in vmd_data.selfShadowFrames:
        mode_str = "Unknown"

        match shadow_frame.mode:
            case vmdio.SelfShadowMode.NoSelfShadow:
                mode_str = "No Self Shadow"
            case vmdio.SelfShadowMode.Mode1:
                mode_str = "Mode 1"
            case vmdio.SelfShadowMode.Mode2:
                mode_str = "Mode 2"

        print(f"Self Shadow Frame: {shadow_frame.frameNumber}")
        print(f"  Shadow Range: {shadow_frame.shadowRange}")
        print(f"  Mode: {mode_str}")
        print()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("vmd_file_path", type=str, help="Path to the camera-edit VMD file to import")
    parser.add_argument("-v", action="store_true", help="Print all frame data of the imported VMD file")
    args = parser.parse_args()

    vmd_file = Path(args.vmd_file_path)

    if not vmd_file.is_file():
        raise FileNotFoundError(f"File not found: {vmd_file}")

    if vmd_file.suffix.lower() != ".vmd":
        raise ValueError(f"Invalid file type: {vmd_file.suffix}. Expected a .vmd file.")

    # Read a camera edit VMD file and populate the camera_edit.VMDData structure
    vmd_data = vmdio.readVMD(vmd_file)

    # Print the summary of the imported data.
    print_summary_data(vmd_data)

    # Print all frames for each type of data if the -v flag is set
    if args.v:
        print_camera_frames(vmd_data)
        print_light_frames(vmd_data)
        print_selfshadow_frames(vmd_data)


if __name__ == "__main__":
    try:
        main()

    # Catch pyvmdio exceptions
    except vmdio_exceptions.VMDIOError as e:
        print(f"vmdio library threw an error: {e}", file=sys.stderr)
        raise SystemExit(1)

    # Catch any other exceptions
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        raise SystemExit(1)