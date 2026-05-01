# model_motion_import.py
# usage:
#    python model_motion_import.py path_to_your_motion.vmd [-v] > output.txt


from pathlib import Path
import argparse
import sys
import pyvmdio.model_edit as vmdio
import pyvmdio.exceptions as vmdio_exceptions

# Set stdout encoding to UTF-8 to avoid encoding issues when printing Japanese text
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")


def print_summary_data(vmd_data: vmdio.VMDData) -> None:
    print("\n=== Summary of imported data ===")
    print(f"Model Name: {vmd_data.modelName.toUTF8ForDisplay()}")
    print(f"Motion Frame Count: {len(vmd_data.motionFrames)}")
    print(f"Morph Frame Count: {len(vmd_data.morphFrames)}")
    print(f"Visible IK Frame Count: {len(vmd_data.visibleIKFrames)}")

    # Note:
    # Frame lists such as motionFrames, morphFrames, visibleIKFrames, and ikDataList
    # are list-like wrappers around C++ std::vector.
    # You can iterate over them and append frames, but do not keep a frame object
    # obtained from a list while changing the same list with append/insert/remove/clear.
    # After changing the list, get the frame again from the list.


def print_motion_frames(vmd_data: vmdio.VMDData) -> None:
    if len(vmd_data.motionFrames) == 0:
        return

    print("\n=== Motion Frames ===")

    for motion_frame in vmd_data.motionFrames:
        pos = motion_frame.position
        rot = motion_frame.rotation
        interp = motion_frame.interpolation

        print(f"Motion Frame: {motion_frame.frameNumber}")
        print(f"  Bone Name: {motion_frame.boneName.toUTF8ForDisplay()}")
        print(f"  Position: ({pos.x}, {pos.y}, {pos.z})")
        print(f"  Rotation: ({rot.qx}, {rot.qy}, {rot.qz}, {rot.qw})")
        print("  Interpolation:")
        print(f"    X Position: ({interp.xPos.x1}, {interp.xPos.y1}, {interp.xPos.x2}, {interp.xPos.y2})")
        print(f"    Y Position: ({interp.yPos.x1}, {interp.yPos.y1}, {interp.yPos.x2}, {interp.yPos.y2})")
        print(f"    Z Position: ({interp.zPos.x1}, {interp.zPos.y1}, {interp.zPos.x2}, {interp.zPos.y2})")
        print(f"    Rotation:   ({interp.rot.x1}, {interp.rot.y1}, {interp.rot.x2}, {interp.rot.y2})")
        print()


def print_morph_frames(vmd_data: vmdio.VMDData) -> None:
    if len(vmd_data.morphFrames) == 0:
        return

    print("\n=== Morph Frames ===")

    for morph_frame in vmd_data.morphFrames:
        print(f"Morph Frame: {morph_frame.frameNumber}")
        print(f"  Morph Name: {morph_frame.morphName.toUTF8ForDisplay()}")
        print(f"  Value: {morph_frame.value}")
        print()


def print_visible_ik_frames(vmd_data: vmdio.VMDData) -> None:
    if len(vmd_data.visibleIKFrames) == 0:
        return

    print("\n=== Visible IK Frames ===")

    for visible_ik_frame in vmd_data.visibleIKFrames:
        visibility_str = "Unknown"

        match visible_ik_frame.visibility:
            case vmdio.Visibility.Visible:
                visibility_str = "Visible"
            case vmdio.Visibility.Hidden:
                visibility_str = "Hidden"

        print(f"Visible IK Frame: {visible_ik_frame.frameNumber}")
        print(f"  Visibility: {visibility_str}")

        if len(visible_ik_frame.ikDataList) == 0:
            continue

        print("  IK Data:")

        for ik_data in visible_ik_frame.ikDataList:
            ik_state_str = "Unknown"

            match ik_data.ikState:
                case vmdio.IKState.ON:
                    ik_state_str = "ON"
                case vmdio.IKState.OFF:
                    ik_state_str = "OFF"

            print(
                f"    IK Bone Name: {ik_data.ikBoneName.toUTF8ForDisplay()}, "
                f"IK State: {ik_state_str}"
            )

        print()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("vmd_file_path", type=str, help="Path to the model edit VMD file to import")
    parser.add_argument("-v", action="store_true", help="Print all frame data of the imported VMD file")
    args = parser.parse_args()

    vmd_file = Path(args.vmd_file_path)

    if not vmd_file.is_file():
        raise FileNotFoundError(f"File not found: {vmd_file}")

    if vmd_file.suffix.lower() != ".vmd":
        raise ValueError(f"Invalid file type: {vmd_file.suffix}. Expected a .vmd file.")

    # Read a model edit VMD file and populate the model_edit.VMDData structure
    vmd_data = vmdio.readVMD(vmd_file)

    # Print the summary of the imported data
    print_summary_data(vmd_data)

    # Print all frames for each type of data if the -v flag is set
    if args.v:
        print_motion_frames(vmd_data)
        print_morph_frames(vmd_data)
        print_visible_ik_frames(vmd_data)


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