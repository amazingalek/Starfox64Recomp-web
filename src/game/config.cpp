// The settings: RecompFrontend's General, Controls, Graphics, Sound and Mods tabs, with its default bindings. Star
// Fox 64's own options are where upstream's menu had them, with its defaults: film grain, the radio communication
// box and the inverted Y axis on the General tab, the music, effects and voice volumes on the Sound tab.

#include <filesystem>
#include <string>
#include <variant>

#include "recompui/config.h"
#include "util/file.h"
#include "sf64.h"

namespace sf64::configkeys {
    namespace general {
        inline const std::string film_grain_mode = "film_grain_mode";
        inline const std::string radio_comm_box_mode = "radio_comm_box_mode";
        inline const std::string invert_y_axis_mode = "invert_y_axis_mode";
    }
    namespace sound {
        inline const std::string bgm_volume = "bgm_volume";
        inline const std::string sfx_volume = "sfx_volume";
        inline const std::string voice_volume = "voice_volume";
    }
}

namespace {
    enum class OnOff : uint32_t { On, Off };
    enum class RadioBoxMode : uint32_t { Expand, Original };

    int get_sound_volume(const std::string& option_id) {
        return static_cast<int>(std::get<double>(recompui::config::get_sound_config().get_option_value(option_id)));
    }

    uint32_t get_general_enum(const std::string& option_id) {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(option_id));
    }
}

int sf64::get_bgm_volume() {
    return get_sound_volume(sf64::configkeys::sound::bgm_volume);
}

int sf64::get_sfx_volume() {
    return get_sound_volume(sf64::configkeys::sound::sfx_volume);
}

int sf64::get_voice_volume() {
    return get_sound_volume(sf64::configkeys::sound::voice_volume);
}

bool sf64::get_film_grain_enabled() {
    return get_general_enum(sf64::configkeys::general::film_grain_mode) == static_cast<uint32_t>(OnOff::On);
}

bool sf64::get_radio_comm_box_expanded() {
    return get_general_enum(sf64::configkeys::general::radio_comm_box_mode) == static_cast<uint32_t>(RadioBoxMode::Expand);
}

bool sf64::get_invert_y_axis_enabled() {
    return get_general_enum(sf64::configkeys::general::invert_y_axis_mode) == static_cast<uint32_t>(OnOff::On);
}

void sf64::init_config() {
    std::filesystem::path app_dir = recompui::file::get_app_folder_path();
    if (!app_dir.empty()) {
        std::filesystem::create_directories(app_dir);
    }

    recompui::config::GeneralTabOptions general_options{};
    general_options.has_rumble_strength = true;
    general_options.has_gyro_sensitivity = true;
    general_options.has_mouse_sensitivity = true;
    recomp::config::Config& general_config = recompui::config::create_general_tab(general_options);
    const std::vector<recomp::config::ConfigOptionEnumOption> on_off = {
        {OnOff::On, "On", "On"},
        {OnOff::Off, "Off", "Off"},
    };
    general_config.add_enum_option(
        sf64::configkeys::general::film_grain_mode,
        "Film Grain",
        "Draws the console's film grain, a faint noise, over the picture.",
        on_off,
        OnOff::On
    );
    general_config.add_enum_option(
        sf64::configkeys::general::radio_comm_box_mode,
        "Radio Communication Box",
        "Where the radio messages are drawn on a wide screen: <recomp-color primary>Original</recomp-color> keeps "
        "the box where the console has it, <recomp-color primary>Expand</recomp-color> moves it to the screen's left edge.",
        {
            {RadioBoxMode::Original, "Original", "Original"},
            {RadioBoxMode::Expand, "Expand", "Expand"},
        },
        RadioBoxMode::Original
    );
    general_config.add_enum_option(
        sf64::configkeys::general::invert_y_axis_mode,
        "Invert Y Axis",
        "Pushing up on the stick points the nose down, as in the original game.",
        on_off,
        OnOff::On
    );
    recompui::config::create_controls_tab();
    recompui::config::create_graphics_tab();

    recomp::config::Config& sound_config = recompui::config::create_sound_tab();
    sound_config.add_percent_number_option(
        sf64::configkeys::sound::bgm_volume,
        "Music Volume",
        "Controls the volume of the background music.",
        100.0f
    );
    sound_config.add_percent_number_option(
        sf64::configkeys::sound::sfx_volume,
        "Sound Effects Volume",
        "Controls the volume of the sound effects.",
        100.0f
    );
    sound_config.add_percent_number_option(
        sf64::configkeys::sound::voice_volume,
        "Voice Volume",
        "Controls the volume of the radio voices.",
        100.0f
    );

    recompui::config::create_mods_tab();
    recompui::config::finalize();
}
