//
// Created by leo on 8/13/25.
//

#ifndef MODELCOMPONENT_HPP
#define MODELCOMPONENT_HPP
#include "Component.hpp"
#include "resources/Model.hpp"

namespace wrld::cpt {
    /// Defines a model for an entity.
    /// This model will be used to render the entity in the virtual world.
    class StaticModel final : public Component {
    public:
        StaticModel(EntityID entity_id, World &world, const Model &model);

        const Model &get_model() const;
        void set_model(const Model &model);

    private:
        Model model;
    };
} // namespace wrld::cpt

#endif // MODELCOMPONENT_HPP
