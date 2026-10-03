#pragma once


#include "inc/types.hpp"
#include "inc/utils.hpp"
#include <cstdint>

class world;

class entity {
public:
    entity(net_uuid uuid);
    virtual ~entity();
private:
    static uint32_t entity_id_counter;
public:
    virtual void tick() = 0;
public:
    uint32_t id;
    net_uuid uuid;
    world *dimension; /* nullable */

    vec3 pos, vel, prev;

    float yaw, pitch;
    bool on_ground;

    int32_t chunk_x, chunk_z;
public:
    bool operator== (const entity &other) const {
        return id == other.id;
    }

    bool operator== (uint32_t other_id) const {
        return id == other_id;
    }
};

namespace std {
    template <>
    struct hash<entity> {
        std::size_t operator() (const entity &e)
        const noexcept {
            return  std::hash<uint32_t>{}(e.id);
        }
    };
}
