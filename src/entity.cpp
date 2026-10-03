#include "inc/entity.hpp"

entity::entity(net_uuid uuid):
    id(entity_id_counter++),
    uuid(uuid)
{}

entity::~entity() = default;

uint32_t entity::entity_id_counter = 1;

