#include "zelda_config.h"
#include "zelda_sound.h"

#include "recompinput/input_mapping.h"
#include "recompinput/input_types.h"
#include "recompui/config.h"
#include "recompui/recompui.h"
#include "util/file.h"

#include <filesystem>

namespace {
constexpr const char* invert_y_axis_mode_option = "invert_y_axis_mode";
constexpr const char* analog_camera_invert_mode_option = "analog_camera_invert_mode";
constexpr const char* two_player_split_screen_direction_option = "two_player_split_screen_direction";

enum class TwoPlayerSplitScreenDirection : uint32_t {
    Horizontal,
    Vertical,
};

const std::vector<recomp::config::ConfigOptionEnumOption> two_player_split_screen_direction_options = {
    {TwoPlayerSplitScreenDirection::Horizontal, "Horizontal"},
    {TwoPlayerSplitScreenDirection::Vertical, "Vertical"},
};

constexpr const char* bgm_volume_option = "bgm_volume";
constexpr const char* sfx_volume_option = "sfx_volume";
constexpr const char* voice_volume_option = "voice_volume";

template <typename T = uint32_t>
T get_general_enum_or_default(const std::string& option_id, T default_value) {
    auto& config = recompui::config::get_general_config();
    if (!config.has_option(option_id)) {
        return default_value;
    }

    return static_cast<T>(std::get<uint32_t>(config.get_option_value(option_id)));
}

template <typename T = uint32_t>
T get_sound_number_or_default(const std::string& option_id, T default_value) {
    auto& config = recompui::config::get_sound_config();
    if (!config.has_option(option_id)) {
        return default_value;
    }

    return static_cast<T>(std::get<double>(config.get_option_value(option_id)));
}

void set_general_enum(const std::string& option_id, uint32_t value) {
    auto& config = recompui::config::get_general_config();
    if (config.has_option(option_id)) {
        config.set_option_value(option_id, value);
    }
}

void set_sound_number(const std::string& option_id, int value) {
    auto& config = recompui::config::get_sound_config();
    if (config.has_option(option_id)) {
        config.set_option_value(option_id, static_cast<double>(value));
    }
}

void set_controller_defaults() {
    using recompinput::GameInput;
    using recompinput::InputField;
    using recompinput::set_default_mapping_for_controller;

    // Accept the adapter's B output as well as the standard gamepad X binding.
    set_default_mapping_for_controller(GameInput::B, {
        InputField::controller_digital(SDL_CONTROLLER_BUTTON_X),
        InputField::controller_digital(SDL_CONTROLLER_BUTTON_B),
    });
    set_default_mapping_for_controller(GameInput::BACK_MENU, {
        InputField::controller_digital(SDL_CONTROLLER_BUTTON_X),
        InputField::controller_digital(SDL_CONTROLLER_BUTTON_B),
    });

    // Keep C buttons on the right stick so face buttons do not activate two N64 inputs.
    set_default_mapping_for_controller(GameInput::C_LEFT, {
        InputField::controller_analog(SDL_CONTROLLER_AXIS_RIGHTX, false),
    });
    set_default_mapping_for_controller(GameInput::C_RIGHT, {
        InputField::controller_analog(SDL_CONTROLLER_AXIS_RIGHTX, true),
    });
    set_default_mapping_for_controller(GameInput::C_UP, {
        InputField::controller_analog(SDL_CONTROLLER_AXIS_RIGHTY, false),
    });
    set_default_mapping_for_controller(GameInput::C_DOWN, {
        InputField::controller_analog(SDL_CONTROLLER_AXIS_RIGHTY, true),
    });
}

void set_control_descriptions() {
    recompinput::set_game_input_description(recompinput::GameInput::Y_AXIS_POS, "Move Up / Menu Up");
    recompinput::set_game_input_description(recompinput::GameInput::Y_AXIS_NEG, "Move Down / Menu Down");
    recompinput::set_game_input_description(recompinput::GameInput::X_AXIS_NEG, "Move Left / Menu Left");
    recompinput::set_game_input_description(recompinput::GameInput::X_AXIS_POS, "Move Right / Menu Right");

    recompinput::set_game_input_description(recompinput::GameInput::A, "Confirm");
    recompinput::set_game_input_description(recompinput::GameInput::B, "Cancel");
    recompinput::set_game_input_description(recompinput::GameInput::Z, "Trick");
    recompinput::set_game_input_description(recompinput::GameInput::L, "Camera");
    recompinput::set_game_input_description(recompinput::GameInput::R, "Crouch");
    recompinput::set_game_input_description(recompinput::GameInput::START, "Pause");

    recompinput::set_game_input_description(recompinput::GameInput::C_UP, "C Up");
    recompinput::set_game_input_description(recompinput::GameInput::C_DOWN, "C Down");
    recompinput::set_game_input_description(recompinput::GameInput::C_LEFT, "C Left");
    recompinput::set_game_input_description(recompinput::GameInput::C_RIGHT, "C Right");
}
}

void zelda64::init_config() {
    set_controller_defaults();

    std::filesystem::path recomp_dir = recompui::file::get_app_folder_path();
    if (!recomp_dir.empty()) {
        std::filesystem::create_directories(recomp_dir);
    }

    recompui::config::GeneralTabOptions general_options{};
    general_options.has_rumble_strength = true;
    general_options.has_gyro_sensitivity = false;
    general_options.has_mouse_sensitivity = false;

    auto& general_config = recompui::config::create_general_tab(general_options);
    general_config.add_enum_option(
        two_player_split_screen_direction_option,
        "2-Player Split-Screen Direction",
        "Sets whether two-player races use a top/bottom or left/right split.",
        two_player_split_screen_direction_options,
        TwoPlayerSplitScreenDirection::Vertical
    );

    recompui::config::create_graphics_tab();

    set_control_descriptions();
    recompui::config::create_controls_tab();

    recompui::config::create_sound_tab();

    recompui::config::create_mods_tab();

    recompui::config::finalize();
}

bool zelda64::get_debug_mode_enabled() {
    return recompui::config::general::get_debug_mode_enabled();
}

bool zelda64::get_vertical_2p_split_screen_enabled() {
    auto direction = get_general_enum_or_default<TwoPlayerSplitScreenDirection>(
        two_player_split_screen_direction_option, TwoPlayerSplitScreenDirection::Vertical);
    return direction == TwoPlayerSplitScreenDirection::Vertical;
}

void zelda64::set_debug_mode_enabled(bool enabled) {
    recompui::config::get_general_config().set_option_value(recompui::config::general::options::debug_mode, enabled);
}

zelda64::AimInvertMode zelda64::get_analog_camera_invert_mode() {
    return get_general_enum_or_default<zelda64::AimInvertMode>(analog_camera_invert_mode_option, zelda64::AimInvertMode::On);
}

void zelda64::set_analog_camera_invert_mode(AimInvertMode mode) {
    set_general_enum(analog_camera_invert_mode_option, static_cast<uint32_t>(mode));
}

zelda64::AimInvertMode zelda64::get_invert_y_axis_mode() {
    return get_general_enum_or_default<zelda64::AimInvertMode>(invert_y_axis_mode_option, zelda64::AimInvertMode::On);
}

void zelda64::set_invert_y_axis_mode(AimInvertMode mode) {
    set_general_enum(invert_y_axis_mode_option, static_cast<uint32_t>(mode));
}

void zelda64::reset_sound_settings() {
    set_main_volume(100);
    set_bgm_volume(100);
    set_sfx_volume(100);
    set_voice_volume(100);
}

void zelda64::set_main_volume(int volume) {
    set_sound_number(recompui::config::sound::options::main_volume, volume);
}

int zelda64::get_main_volume() {
    return get_sound_number_or_default<int>(recompui::config::sound::options::main_volume, 100);
}

void zelda64::set_bgm_volume(int volume) {
    set_sound_number(bgm_volume_option, volume);
}

void zelda64::set_sfx_volume(int volume) {
    set_sound_number(sfx_volume_option, volume);
}

void zelda64::set_voice_volume(int volume) {
    set_sound_number(voice_volume_option, volume);
}

int zelda64::get_bgm_volume() {
    return get_sound_number_or_default<int>(bgm_volume_option, 100);
}

int zelda64::get_sfx_volume() {
    return get_sound_number_or_default<int>(sfx_volume_option, 100);
}

int zelda64::get_voice_volume() {
    return get_sound_number_or_default<int>(voice_volume_option, 100);
}
