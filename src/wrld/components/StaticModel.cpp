//
// Created by leo on 8/13/25.
//

#include <wrld/components/StaticModel.hpp>

namespace wrld::cpt {
    StaticModel::StaticModel(const EntityID entity_id, World &world, const Rc<rsc::Model> &model) :
        Component(entity_id, world) {
        attach_resource("model", model);
    }

    Rc<rsc::Model> StaticModel::get_model() const { return get_resource<rsc::Model>("model"); }

    void StaticModel::set_model(const Rc<rsc::Model> &model) { attach_resource("model", model); }
} // namespace wrld::cpt
