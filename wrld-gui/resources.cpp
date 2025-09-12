//
// Created by leo on 9/12/25.
//

#include "resources.hpp"

#include "imgui.h"

namespace wrld::gui {
    void render_resources_window(const World &world) {
        ImGui::Begin("Resources");

        for (const auto &pool: world.get_resources() | std::views::values) {
            if (pool.empty())
                continue;

            const std::string &type_name = pool.begin()->second->get_type();

            if (ImGui::TreeNode(type_name.c_str())) {
                for (const auto &key: pool | std::views::keys) {
                    ImGui::Text("%s", key.c_str());
                }
                ImGui::TreePop();
            }
        }

        ImGui::End();
    }
} // namespace wrld::gui
