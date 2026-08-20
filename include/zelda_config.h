#ifndef __ZELDA_CONFIG_H__
#define __ZELDA_CONFIG_H__

#include <filesystem>
#include <string_view>
#include "ultramodern/config.hpp"

namespace zelda64 {
    constexpr std::u8string_view program_id = u8"SnowboardKidsRecompiled";
    constexpr std::string_view program_name = "Snowboard Kids: Recompiled";

    void init_config();
    
    bool get_debug_mode_enabled();
    void set_debug_mode_enabled(bool enabled);
    bool get_vertical_2p_split_screen_enabled();
    
    enum class AimInvertMode {
        On,
        Off,
        OptionCount
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(zelda64::AimInvertMode, {
        {zelda64::AimInvertMode::On, "On"},
        {zelda64::AimInvertMode::Off, "Off"},
    });

    AimInvertMode get_analog_camera_invert_mode();
    void set_analog_camera_invert_mode(AimInvertMode mode);

    AimInvertMode get_invert_y_axis_mode();
    void set_invert_y_axis_mode(AimInvertMode mode);

    void open_quit_game_prompt();
};

#endif
