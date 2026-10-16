#ifndef __SF64_H__
#define __SF64_H__

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace sf64 {
    inline constexpr std::u8string_view program_id = u8"Starfox64Recompiled";
    inline constexpr std::string_view program_name = "Starfox 64: Recompiled";

    void register_overlays();
    void register_patches();

    // The ROM's MIO0-compressed files decompressed in place, for the code the game was recompiled from
    // (rom_decompression.cpp).
    std::vector<uint8_t> decompress_rom(std::span<const uint8_t> compressed_rom);

    // The settings (config.cpp): RecompFrontend's tabs, with the game's own options on the General tab and the
    // music, effects and voice volumes on the Sound tab, as upstream's menu had them.
    void init_config();
    int get_bgm_volume();
    int get_sfx_volume();
    int get_voice_volume();
    bool get_film_grain_enabled();
    bool get_radio_comm_box_expanded();
    bool get_invert_y_axis_enabled();
}

#endif
