#pragma once

#include <cstdint>
#include <unordered_map>
#include <array>

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

struct chunk {
    std::array<chunk_section, 384 / 16> sections;
};

class chunk_loader {
private:
    chunk_loader();
public:
    static chunk_loader instance;
public:
    chunk &load_chunk(int32_t cx, int32_t cy);
private:
    chunk_section uniform_section (
        uint32_t block,
        uint32_t biome,
        uint8_t light
    );
private:
    std::unordered_map<
        std::pair<int32_t, int32_t>,
        chunk
    > loaded_chunks;
};
