#pragma once

#include <cstdint>
#include <string_view>

namespace vmdio::internal
{
    inline constexpr uint32_t MAX_FRAME_COUNT = 600000;

    inline constexpr uint32_t VMD_HEADER_SIZE = 30;
    inline constexpr uint32_t VMD_MODEL_NAME_HEADER_SIZE = 20;
    inline constexpr uint32_t VMD_BONE_NAME_FIELD_SIZE = 15;
    inline constexpr uint32_t VMD_MORPH_NAME_FIELD_SIZE = 15;
    inline constexpr uint32_t VMD_IK_BONE_NAME_FIELD_SIZE = 20;
    inline constexpr uint32_t VMD_MOTION_INTERPOLATION_FIELD_SIZE = 64;
    inline constexpr uint32_t VMD_CAMERA_INTERPOLATION_FIELD_SIZE = 24;

    inline constexpr uint32_t VMD_CAMERA_FRAME_SIZE = 61;
    inline constexpr uint32_t VMD_LIGHT_FRAME_SIZE = 28;
    inline constexpr uint32_t VMD_SELF_SHADOW_FRAME_SIZE = 9;
    inline constexpr uint32_t VMD_MOTION_FRAME_SIZE = 111;
    inline constexpr uint32_t VMD_MORPH_FRAME_SIZE = 23;

    inline constexpr std::string_view VMD_HEADER = "Vocaloid Motion Data 0002";
    inline constexpr std::string_view VMD_MODEL_NAME_CAMERA_EDIT = "カメラ・照明";
}