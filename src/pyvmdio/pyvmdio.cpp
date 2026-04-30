#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl/filesystem.h>

#include <string>
#include <string_view>

#include "vmdio/camera_edit.h"
#include "vmdio/model_edit.h"
#include "vmdio/vmd_encoding.h"
#include "vmdio/vmd_exceptions.h"
#include "vmdio/vmd_string.h"

namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::vector<vmdio::camera_edit::CameraFrame>);
PYBIND11_MAKE_OPAQUE(std::vector<vmdio::camera_edit::LightFrame>);
PYBIND11_MAKE_OPAQUE(std::vector<vmdio::camera_edit::SelfShadowFrame>);
PYBIND11_MAKE_OPAQUE(std::vector<vmdio::model_edit::MotionFrame>);
PYBIND11_MAKE_OPAQUE(std::vector<vmdio::model_edit::MorphFrame>);
PYBIND11_MAKE_OPAQUE(std::vector<vmdio::model_edit::VisibleIKFrame>);
PYBIND11_MAKE_OPAQUE(std::vector<vmdio::model_edit::IKData>);

namespace
{
    py::bytes stdStringToPythonBytes(const std::string &pBytes)
    {
        return py::bytes(pBytes.data(), pBytes.size());
    }

    vmdio::VMDString toVMDString(const py::object &pValue)
    {
        if (py::isinstance<vmdio::VMDString>(pValue))
            return pValue.cast<vmdio::VMDString>();

        if (py::isinstance<py::str>(pValue))
        {
            const std::string lUTF8String = pValue.cast<std::string>();
            return vmdio::VMDString::fromUTF8(lUTF8String);
        }

        if (py::isinstance<py::bytes>(pValue))
        {
            const std::string lUTF8Bytes = pValue.cast<std::string>();
            return vmdio::VMDString::fromUTF8(lUTF8Bytes);
        }

        throw py::type_error("Expected VMDString, str, or UTF-8 bytes");
    }

    void bindCameraEdit(py::module_ &pModule)
    {
        py::enum_<vmdio::camera_edit::ProjectionType>(pModule, "ProjectionType")
            .value("Perspective", vmdio::camera_edit::ProjectionType::Perspective)
            .value("Orthographic", vmdio::camera_edit::ProjectionType::Orthographic);

        py::enum_<vmdio::camera_edit::SelfShadowMode>(pModule, "SelfShadowMode")
            .value("NoSelfShadow", vmdio::camera_edit::SelfShadowMode::NoSelfShadow)
            .value("Mode1", vmdio::camera_edit::SelfShadowMode::Mode1)
            .value("Mode2", vmdio::camera_edit::SelfShadowMode::Mode2);

        py::class_<vmdio::camera_edit::Position>(pModule, "Position")
            .def(py::init<>())
            .def_readwrite("x", &vmdio::camera_edit::Position::x)
            .def_readwrite("y", &vmdio::camera_edit::Position::y)
            .def_readwrite("z", &vmdio::camera_edit::Position::z);

        py::class_<vmdio::camera_edit::CameraRotation>(pModule, "CameraRotation")
            .def(py::init<>())
            .def_readwrite("eulerX", &vmdio::camera_edit::CameraRotation::eulerX)
            .def_readwrite("eulerY", &vmdio::camera_edit::CameraRotation::eulerY)
            .def_readwrite("eulerZ", &vmdio::camera_edit::CameraRotation::eulerZ);

        py::class_<vmdio::camera_edit::ControlPointSet>(pModule, "ControlPointSet")
            .def(py::init<>())
            .def_readwrite("x1", &vmdio::camera_edit::ControlPointSet::x1)
            .def_readwrite("y1", &vmdio::camera_edit::ControlPointSet::y1)
            .def_readwrite("x2", &vmdio::camera_edit::ControlPointSet::x2)
            .def_readwrite("y2", &vmdio::camera_edit::ControlPointSet::y2);

        py::class_<vmdio::camera_edit::CameraInterpolation>(pModule, "CameraInterpolation")
            .def(py::init<>())
            .def_readwrite("xPos", &vmdio::camera_edit::CameraInterpolation::xPos)
            .def_readwrite("yPos", &vmdio::camera_edit::CameraInterpolation::yPos)
            .def_readwrite("zPos", &vmdio::camera_edit::CameraInterpolation::zPos)
            .def_readwrite("rot", &vmdio::camera_edit::CameraInterpolation::rot)
            .def_readwrite("dist", &vmdio::camera_edit::CameraInterpolation::dist)
            .def_readwrite("view", &vmdio::camera_edit::CameraInterpolation::view);

        py::class_<vmdio::camera_edit::Color>(pModule, "Color")
            .def(py::init<>())
            .def_readwrite("r", &vmdio::camera_edit::Color::r)
            .def_readwrite("g", &vmdio::camera_edit::Color::g)
            .def_readwrite("b", &vmdio::camera_edit::Color::b);

        py::class_<vmdio::camera_edit::CameraFrame>(pModule, "CameraFrame")
            .def(py::init<>())
            .def_readwrite("frameNumber", &vmdio::camera_edit::CameraFrame::frameNumber)
            .def_readwrite("distance", &vmdio::camera_edit::CameraFrame::distance)
            .def_readwrite("position", &vmdio::camera_edit::CameraFrame::position)
            .def_readwrite("rotation", &vmdio::camera_edit::CameraFrame::rotation)
            .def_readwrite("interpolation", &vmdio::camera_edit::CameraFrame::interpolation)
            .def_readwrite("viewingAngle", &vmdio::camera_edit::CameraFrame::viewingAngle)
            .def_readwrite("projectionType", &vmdio::camera_edit::CameraFrame::projectionType);

        py::class_<vmdio::camera_edit::LightFrame>(pModule, "LightFrame")
            .def(py::init<>())
            .def_readwrite("frameNumber", &vmdio::camera_edit::LightFrame::frameNumber)
            .def_readwrite("color", &vmdio::camera_edit::LightFrame::color)
            .def_readwrite("position", &vmdio::camera_edit::LightFrame::position);

        py::class_<vmdio::camera_edit::SelfShadowFrame>(pModule, "SelfShadowFrame")
            .def(py::init<>())
            .def_readwrite("frameNumber", &vmdio::camera_edit::SelfShadowFrame::frameNumber)
            .def_readwrite("mode", &vmdio::camera_edit::SelfShadowFrame::mode)
            .def_readwrite("shadowRange", &vmdio::camera_edit::SelfShadowFrame::shadowRange);

        py::bind_vector<std::vector<vmdio::camera_edit::CameraFrame>>(
            pModule, "CameraFrameList");

        py::bind_vector<std::vector<vmdio::camera_edit::LightFrame>>(
            pModule, "LightFrameList");

        py::bind_vector<std::vector<vmdio::camera_edit::SelfShadowFrame>>(
            pModule, "SelfShadowFrameList");

        py::class_<vmdio::camera_edit::VMDData>(pModule, "VMDData")
            .def(py::init<>())
            .def_readwrite("cameraFrames", &vmdio::camera_edit::VMDData::cameraFrames)
            .def_readwrite("lightFrames", &vmdio::camera_edit::VMDData::lightFrames)
            .def_readwrite("selfShadowFrames", &vmdio::camera_edit::VMDData::selfShadowFrames);

        pModule.def("readVMD", &vmdio::camera_edit::readVMD,
                    "Read a VMD file and return a CameraEdit.VMDData object",
                    py::arg("file_path"),
                    py::call_guard<py::gil_scoped_release>());

        pModule.def("writeVMD", &vmdio::camera_edit::writeVMD,
                    "Write a CameraEdit.VMDData object to a VMD file",
                    py::arg("vmd_data"),
                    py::arg("file_path"),
                    py::call_guard<py::gil_scoped_release>());
    }

    void bindModelEdit(py::module_ &pModule)
    {
        py::enum_<vmdio::model_edit::IKState>(pModule, "IKState")
            .value("OFF", vmdio::model_edit::IKState::OFF)
            .value("ON", vmdio::model_edit::IKState::ON);

        py::enum_<vmdio::model_edit::Visibility>(pModule, "Visibility")
            .value("Hidden", vmdio::model_edit::Visibility::Hidden)
            .value("Visible", vmdio::model_edit::Visibility::Visible);

        py::class_<vmdio::model_edit::Position>(pModule, "Position")
            .def(py::init<>())
            .def_readwrite("x", &vmdio::model_edit::Position::x)
            .def_readwrite("y", &vmdio::model_edit::Position::y)
            .def_readwrite("z", &vmdio::model_edit::Position::z);

        py::class_<vmdio::model_edit::Quaternion>(pModule, "Quaternion")
            .def(py::init<>())
            .def_readwrite("qx", &vmdio::model_edit::Quaternion::qx)
            .def_readwrite("qy", &vmdio::model_edit::Quaternion::qy)
            .def_readwrite("qz", &vmdio::model_edit::Quaternion::qz)
            .def_readwrite("qw", &vmdio::model_edit::Quaternion::qw);

        py::class_<vmdio::model_edit::ControlPointSet>(pModule, "ControlPointSet")
            .def(py::init<>())
            .def_readwrite("x1", &vmdio::model_edit::ControlPointSet::x1)
            .def_readwrite("y1", &vmdio::model_edit::ControlPointSet::y1)
            .def_readwrite("x2", &vmdio::model_edit::ControlPointSet::x2)
            .def_readwrite("y2", &vmdio::model_edit::ControlPointSet::y2);

        py::class_<vmdio::model_edit::MotionInterpolation>(pModule, "MotionInterpolation")
            .def(py::init<>())
            .def_readwrite("xPos", &vmdio::model_edit::MotionInterpolation::xPos)
            .def_readwrite("yPos", &vmdio::model_edit::MotionInterpolation::yPos)
            .def_readwrite("zPos", &vmdio::model_edit::MotionInterpolation::zPos)
            .def_readwrite("rot", &vmdio::model_edit::MotionInterpolation::rot);

        py::class_<vmdio::model_edit::MotionFrame>(pModule, "MotionFrame")
            .def(py::init<>())
            .def_property(
                "boneName",
                [](const vmdio::model_edit::MotionFrame &pFrame)
                {
                    return pFrame.boneName;
                },
                [](vmdio::model_edit::MotionFrame &pFrame, const py::object &pValue)
                {
                    pFrame.boneName = toVMDString(pValue);
                })
            .def_readwrite("frameNumber", &vmdio::model_edit::MotionFrame::frameNumber)
            .def_readwrite("position", &vmdio::model_edit::MotionFrame::position)
            .def_readwrite("rotation", &vmdio::model_edit::MotionFrame::rotation)
            .def_readwrite("interpolation", &vmdio::model_edit::MotionFrame::interpolation);

        py::class_<vmdio::model_edit::MorphFrame>(pModule, "MorphFrame")
            .def(py::init<>())
            .def_property(
                "morphName",
                [](const vmdio::model_edit::MorphFrame &pFrame)
                {
                    return pFrame.morphName;
                },
                [](vmdio::model_edit::MorphFrame &pFrame, const py::object &pValue)
                {
                    pFrame.morphName = toVMDString(pValue);
                })
            .def_readwrite("frameNumber", &vmdio::model_edit::MorphFrame::frameNumber)
            .def_readwrite("value", &vmdio::model_edit::MorphFrame::value);

        py::class_<vmdio::model_edit::IKData>(pModule, "IKData")
            .def(py::init<>())
            .def_property(
                "ikBoneName",
                [](const vmdio::model_edit::IKData &pIKData)
                {
                    return pIKData.ikBoneName;
                },
                [](vmdio::model_edit::IKData &pIKData, const py::object &pValue)
                {
                    pIKData.ikBoneName = toVMDString(pValue);
                })
            .def_readwrite("ikState", &vmdio::model_edit::IKData::ikState);

        py::bind_vector<std::vector<vmdio::model_edit::IKData>>(pModule, "IKDataList");

        py::class_<vmdio::model_edit::VisibleIKFrame>(pModule, "VisibleIKFrame")
            .def(py::init<>())
            .def_readwrite("frameNumber", &vmdio::model_edit::VisibleIKFrame::frameNumber)
            .def_readwrite("visibility", &vmdio::model_edit::VisibleIKFrame::visibility)
            .def_readwrite("ikDataList", &vmdio::model_edit::VisibleIKFrame::ikDataList);

        py::bind_vector<std::vector<vmdio::model_edit::MotionFrame>>(
            pModule, "MotionFrameList");

        py::bind_vector<std::vector<vmdio::model_edit::MorphFrame>>(
            pModule, "MorphFrameList");

        py::bind_vector<std::vector<vmdio::model_edit::VisibleIKFrame>>(
            pModule, "VisibleIKFrameList");

        py::class_<vmdio::model_edit::VMDData>(pModule, "VMDData")
            .def(py::init<>())
            .def_property(
                "modelName",
                [](const vmdio::model_edit::VMDData &pData)
                {
                    return pData.modelName;
                },
                [](vmdio::model_edit::VMDData &pData, const py::object &pValue)
                {
                    pData.modelName = toVMDString(pValue);
                })
            .def_readwrite("motionFrames", &vmdio::model_edit::VMDData::motionFrames)
            .def_readwrite("morphFrames", &vmdio::model_edit::VMDData::morphFrames)
            .def_readwrite("visibleIKFrames", &vmdio::model_edit::VMDData::visibleIKFrames);

        pModule.def("readVMD", &vmdio::model_edit::readVMD,
                    "Read a VMD file and return a ModelEdit.VMDData object",
                    py::arg("file_path"),
                    py::call_guard<py::gil_scoped_release>());

        pModule.def("writeVMD", &vmdio::model_edit::writeVMD,
                    "Write a ModelEdit.VMDData object to a VMD file",
                    py::arg("vmd_data"),
                    py::arg("file_path"),
                    py::call_guard<py::gil_scoped_release>());
    }

    void bindEncoding(py::module_ &pModule)
    {
        pModule.def(
            "utf8ToShiftJIS",
            [](const std::string &pUTF8String)
            {
                const std::string lShiftJISBytes =
                    vmdio::encoding::utf8ToShiftJIS(pUTF8String);

                return py::bytes(lShiftJISBytes.data(), lShiftJISBytes.size());
            },
            "Convert UTF-8 text or UTF-8 bytes to Shift_JIS bytes.",
            py::arg("utf8_string_or_bytes"));

        pModule.def(
            "shiftJISToUTF8",
            [](py::bytes pShiftJISBytes)
            {
                const std::string lShiftJISBytes = pShiftJISBytes;
                return vmdio::encoding::shiftJISToUTF8(lShiftJISBytes);
            },
            "Convert Shift_JIS bytes to UTF-8 text.",
            py::arg("shift_jis_bytes"));
    }

    void bindExceptions(py::module_ &pModule)
    {
        py::exception<vmdio::exceptions::VMDIOError> lVMDIOError(pModule, "VMDIOError");

        py::register_exception<vmdio::exceptions::FileSystemError>(
            pModule, "FileSystemError", lVMDIOError.ptr());

        py::register_exception<vmdio::exceptions::FrameConflictError>(
            pModule, "FrameConflictError", lVMDIOError.ptr());

        py::register_exception<vmdio::exceptions::FrameOverflowError>(
            pModule, "FrameOverflowError", lVMDIOError.ptr());

        py::register_exception<vmdio::exceptions::IncompatibleFormatError>(
            pModule, "IncompatibleFormatError", lVMDIOError.ptr());

        py::register_exception<vmdio::exceptions::InvalidFieldValueError>(
            pModule, "InvalidFieldValueError", lVMDIOError.ptr());

        py::register_exception<vmdio::exceptions::StringProcessError>(
            pModule, "StringProcessError", lVMDIOError.ptr());
    }

    void bindVMDString(py::module_ &pModule)
    {
        py::class_<vmdio::VMDString>(pModule, "VMDString")
            .def(py::init<>())

            .def("empty", &vmdio::VMDString::empty)

            .def("sizeofShiftJISBytes", &vmdio::VMDString::sizeofShiftJISBytes)

            .def_static(
                "fromShiftJIS",
                [](py::bytes pShiftJISBytes)
                {
                    const std::string lShiftJISBytes = pShiftJISBytes;
                    return vmdio::VMDString::fromShiftJIS(lShiftJISBytes);
                },
                "Create a VMDString from Shift_JIS bytes.",
                py::arg("shift_jis_bytes"))

            .def_static(
                "fromUTF8",
                [](const std::string &pUTF8StringOrBytes)
                {
                    return vmdio::VMDString::fromUTF8(pUTF8StringOrBytes);
                },
                "Create a VMDString from a Python string or UTF-8 bytes.",
                py::arg("utf8_string_or_bytes"))

            .def(
                "toShiftJIS",
                [](const vmdio::VMDString &pString)
                {
                    return stdStringToPythonBytes(pString.toShiftJIS());
                },
                "Return the stored Shift_JIS byte sequence as Python bytes.")

            .def("toUTF8", &vmdio::VMDString::toUTF8)

            .def("toUTF8ForDisplay", &vmdio::VMDString::toUTF8ForDisplay,
                 py::arg("stop_at_nul") = true)

            .def(
                "__bool__",
                [](const vmdio::VMDString &pString)
                {
                    return !pString.empty();
                })

            .def(
                "__eq__",
                [](const vmdio::VMDString &pLeft, const vmdio::VMDString &pRight)
                {
                    return pLeft.toShiftJIS() == pRight.toShiftJIS();
                },
                py::is_operator())

            .def(
                "__hash__",
                [](const vmdio::VMDString &pString)
                {
                    const std::string bytes = pString.toShiftJIS();
                    return py::hash(py::bytes(bytes.data(), bytes.size()));
                })

            .def(
                "__repr__",
                [](const vmdio::VMDString &pString)
                {
                    py::str s(pString.toUTF8ForDisplay());
                    return "<VMDString " + py::repr(s).cast<std::string>() + ">";
                })

            .def(
                "__str__",
                [](const vmdio::VMDString &pString)
                {
                    return pString.toUTF8ForDisplay();
                });
    }
}

PYBIND11_MODULE(_pyvmdio, pModule)
{
    py::module_ lCameraModule = pModule.def_submodule("camera_edit");
    py::module_ lEncodingModule = pModule.def_submodule("encoding");
    py::module_ lExceptionsModule = pModule.def_submodule("exceptions");
    py::module_ lModelModule = pModule.def_submodule("model_edit");
    py::module_ lStringModule = pModule.def_submodule("vmd_string");

    bindVMDString(lStringModule);
    bindExceptions(lExceptionsModule);
    bindEncoding(lEncodingModule);
    bindCameraEdit(lCameraModule);
    bindModelEdit(lModelModule);

    pModule.attr("VMDString") = lStringModule.attr("VMDString");
}