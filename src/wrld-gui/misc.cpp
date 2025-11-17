//
// Created by leo on 10/20/25.
//

#include <imgui.h>
#include <wrld-gui/misc.hpp>

namespace wrld::gui {
    void render_info_window(World &world) {
        const auto &viewport = Main::get_window_viewport();

        ImGui::Begin("Info");
        ImGui::Text("Deltatime: %.2f", Main::get_delta_time());
        ImGui::Text("FPS: %d", static_cast<int>(1.0 / Main::get_delta_time()));
        ImGui::Text("Viewport size: %d x %d", viewport->get_width(),
                    viewport->get_height());
        ImGui::SeparatorText("Statistics");

        for (const auto &[name, value]: Main::get_statistics()) {
            ImGui::Text("%s: %s", name.c_str(), value.c_str());
        }

        ImGui::End();
    }
} // namespace wrld::gui
