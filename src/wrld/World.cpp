//
// Created by leo on 8/11/25.
//

#include <wrld/World.hpp>

#include <random>
#include <ranges>

#include <wrld/resources/Program.hpp>
#include <wrld/resources/Texture.hpp>

namespace wrld {
    World::World() :
        components({}) {
    }

    EntityID World::create_entity(const std::string &name) {
        max_entity_id += 1;
        EntityID res = max_entity_id;

        if (name.empty())
            entities.emplace(res, std::format("#{}", res));
        else
            entities.emplace(res, name);

        return res;
    }

    std::string World::get_entity_name(const EntityID id) const {
        return entities.at(id);
    }

    void World::delete_entity(const EntityID id) {
        if (!entity_exists(id))
            return;

        entities.erase(id);
        for (auto &component_pool: components | std::views::values) {
            component_pool.erase(id);
        }
    }

    std::unordered_map<EntityID, std::string> World::get_entities() const {
        return entities;
    }

    std::vector<std::type_index>
    World::get_components_of_entity(const EntityID id) const {
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
        static std::uniform_int_distribution<size_t> dis(
                1, std::numeric_limits<size_t>::max());
        return dis(gen);
    }

    template<>
    Rc<rsc::Program> World::get_default() {
        if (!default_resources.contains(std::type_index(typeid(rsc::Program)))) {
            const auto new_resource = Rc<rsc::Program>("default", *this);
            new_resource->from_default();
            const Rc<Resource> casted = new_resource.as<Resource>();
            default_resources.insert_or_assign(std::type_index(typeid(rsc::Program)),
                                               casted);
        }

        return default_resources.at(std::type_index(typeid(rsc::Program)))
                                .as<rsc::Program>();
    }

    template<>
    Rc<rsc::Texture> World::get_default() {
        if (!default_resources.contains(std::type_index(typeid(rsc::Texture)))) {
            const auto new_resource = Rc<rsc::Texture>("default", *this);
            new_resource->from_default();
            const Rc<Resource> casted = new_resource.as<Resource>();

            default_resources.insert_or_assign(std::type_index(typeid(rsc::Texture)),
                                               casted);
        }

        return default_resources.at(std::type_index(typeid(rsc::Texture)))
                                .as<rsc::Texture>();
    }
} // namespace wrld
