#pragma once

#include "inc/blocks.hpp"
#include "inc/registry.hpp"

/**
 * holds data for a 16x16x16 portion of a chunk
 */
struct chunk_section {
    std::array<std::array<std::array<uint32_t, 16>, 16>, 16>
        blocks
    ;

    std::array<std::array<std::array<uint32_t, 4>, 4>, 4>
        biomes
    ;

    std::array<std::array<std::array<uint8_t, 16>, 16>, 16>
        lights
    ;

    /**
     * indexed [x][z][y]
     */
};

template <size_t H>
struct chunk {
    std::array<chunk_section, H / 16> sections;
};

template <size_t H>
class chunk_loader {
private:
    using chunk = chunk<H>;
public:
    chunk load_chunk(int64_t x, int64_t y) {
        /**
         * TODO: implement loading from world file & proper caching
         */

        chunk_section all_stone = uniform_section (
            block::stone,
            synced_registries::instance["minecraft:worldgen/biome"]["minecraft:plains"],
            0
        );

        chunk_section all_air = uniform_section (
            block::air,
            synced_registries::instance["minecraft:worldgen/biome"]["minecraft:plains"],
            15
        );

        chunk res;

        std::fill(
            res.sections.begin(),
            res.sections.begin() + 64 / 16,
            all_stone
        );

        std::fill(
            res.sections.begin() + 64 / 16,
            res.sections.end(),
            all_air
        );

        return res;
    }

private:
    struct loaded_chunk {
        int64_t x, y;
        chunk c;
    };

private:
    chunk_section uniform_section (
        uint32_t block,
        uint32_t biome,
        uint8_t light
    ){
        chunk_section section;

        auto fill_cubed = [](
            auto &arr, auto data
        ) -> void {
            std::fill(
                arr[0][0].begin(), arr[0][0].end(), data
            );
            std::fill(
                arr[0].begin(), arr[0].end(), arr[0][0]
            );
            std::fill(
                arr.begin(), arr.end(), arr[0]
            );
        };

        fill_cubed(section.blocks, block);
        fill_cubed(section.biomes, biome);
        fill_cubed(section.lights, light);

        return section;
    }
};

