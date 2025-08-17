//
// Created by leo on 8/13/25.
//

#ifndef MODELCOMPONENT_HPP
#define MODELCOMPONENT_HPP
#include "Component.hpp"
#include "resources/Model.hpp"

namespace wrld {
    /// Defines a model for an entity.
    /// This model will be used to render the entity in the virtual world.
    class ModelComponent final : public Component {
    public:
        ModelComponent(EntityID entity_id, World &world, const Model &model);

        const Model &get_model() const;
        void set_model(const Model &model);

    private:
        Model model;
    };
} // namespace wrld

#endif // MODELCOMPONENT_HPP
