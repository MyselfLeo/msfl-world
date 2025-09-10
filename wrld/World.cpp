//
// Created by leo on 8/11/25.
//

#include "World.hpp"

#include <random>
#include <ranges>

namespace wrld {
    World::World() : components({}) {}

    EntityID World::create_entity(const std::string &name) {
        max_entity_id += 1;
        EntityID res = max_entity_id;

        if (name.empty())
            entities.emplace(res, std::format("#{}", res));
        else
            entities.emplace(res, name);

        return res;
    }

    std::string World::get_entity_name(const EntityID id) { return entities[id]; }

    void World::delete_entity(const EntityID id) {
        if (!entity_exists(id))
            return;

        entities.erase(id);
        for (auto &component_pool: components | std::views::values) {
            component_pool.erase(id);
        }
    }

    std::unordered_map<EntityID, std::string> World::get_entities() const { return entities; }

    std::vector<std::type_index> World::get_components_of_entity(const EntityID id) const {
        std::vector<std::type_index> res;

        for (const auto &[cpt_type, entity_map]: components) {
            if (entity_map.contains(id))
                res.push_back(cpt_type);
        }

        return res;
    }

    const ResourcePool &World::get_resources() const { return resources; }

    bool World::entity_exists(const EntityID id) const { return entities.contains(id); }

    size_t World::generate_random_id() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<size_t> dis(1, std::numeric_limits<size_t>::max());
        return dis(gen);
    }
} // namespace wrld
