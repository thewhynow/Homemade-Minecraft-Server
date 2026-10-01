#include "inc/utils.hpp"
#include "inc/chunk.hpp"
#include "inc/blocks.hpp"
#include "inc/registry.hpp"

#include <algorithm>
#include <unordered_map>

chunk_loader::chunk_loader() = default;

chunk_loader chunk_loader::instance;

chunk_section chunk_loader::uniform_section (
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

chunk &chunk_loader::load_chunk(int32_t cx, int32_t cz) {
    /**
     * TODO: implement loading from world file & proper caching
     */

    if (loaded_chunks.contains({cx, cz}))
        return loaded_chunks[{cx, cz}];

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

    return loaded_chunks[{cx, cz}] = res;
}
