//
// Created by leo on 8/11/25.
//

#include "World.hpp"

#include <random>
#include <ranges>

namespace wrld {
    World::World() : components({}) {}

    EntityID World::create_entity() {
        // Create random ID until one is free
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<size_t> dis(0, SIZE_MAX);

        EntityID res;
        do {
            res = dis(gen);
        } while (exists(res));

        entities.emplace(res);

        return res;
    }

    void World::delete_entity(const EntityID id) {
        if (!exists(id))
            return;

        entities.erase(id);
        for (auto &component_pool: components | std::views::values) {
            component_pool.erase(id);
        }
    }

    bool World::exists(const EntityID id) const { return entities.contains(id); }
} // namespace wrld
