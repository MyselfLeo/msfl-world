//
// Created by leo on 8/11/25.
//

#include "World.hpp"

#include <random>
#include <ranges>

namespace wrld {
    World::World() : components({}) {}

    EntityID World::create_entity() {
        EntityID res;
        do {
            res = generate_random_id();
        } while (entity_exists(res));

        entities.emplace(res);

        return res;
    }

    void World::delete_entity(const EntityID id) {
        if (!entity_exists(id))
            return;

        entities.erase(id);
        for (auto &component_pool: components | std::views::values) {
            component_pool.erase(id);
        }
    }

    const std::unordered_set<EntityID> &World::get_entities() const { return entities; }

    bool World::entity_exists(const EntityID id) const { return entities.contains(id); }

    bool World::resource_exists(const rsc::ResourceID id) const { return resources.contains(id); }

    size_t World::generate_random_id() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<size_t> dis(1, std::numeric_limits<size_t>::max());
        return dis(gen);
    }
} // namespace wrld
