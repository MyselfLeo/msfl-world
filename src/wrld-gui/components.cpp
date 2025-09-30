//
// Created by leo on 9/12/25.
//

#include <wrld-gui/components.hpp>

namespace wrld::gui {
    void render_component_window(World &world, bool *p_open) {
        ImGui::Begin("Scene", p_open);
        for (const auto &[ent_id, ent_name]: world.get_entities()) {
            if (ImGui::TreeNode(std::format("{}##{}", ent_name, ent_id).c_str())) {
                ImGui::Text("%s", std::format("Entity ID: {}", ent_id).c_str());

                auto component_types = world.get_components_of_entity(ent_id);
                for (const auto &type: component_types) {
                    // Retrieve the appropriate function from the map and call it
                    if (COMPONENT_FUNCTIONS.contains(type)) {
                        COMPONENT_FUNCTIONS.at(type)(world, ent_id);
                    }
                }
                ImGui::TreePop();
            }
        }

        ImGui::End();
    }

} // namespace wrld::gui
