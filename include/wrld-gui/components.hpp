//
// Created by leo on 9/8/25.
//

#pragma once

#include <wrld/World.hpp>
#include <wrld/components/Camera3D.hpp>
#include <wrld/components/DirectionalLight.hpp>
#include <wrld/components/Environment.hpp>
#include <wrld/components/FPSControl.hpp>
#include <wrld/components/Orbiter.hpp>
#include <wrld/components/PointLight.hpp>
#include <wrld/components/StaticModel.hpp>
#include <wrld/components/Transform.hpp>

#include <imgui.h>
#include <map>

namespace wrld::gui {
    // Function pointer type for component menu handlers
    using ComponentMenuFunction = void (*)(World &, const EntityID &);

    template<ComponentConcept C>
    void component_menu(World &world, const EntityID &entity) {
        if (const auto &cpt = world.get_component<C>(entity); ImGui::TreeNode(cpt->get_type().c_str())) {
            // Default behavior for any component type
            ImGui::Text("No Action");
            ImGui::TreePop();
        }
    }

    template<>
    inline void component_menu<cpt::Camera3D>(World &world, const EntityID &entity) {
        const auto &cpt = world.get_component<cpt::Camera3D>(entity);
        if (ImGui::TreeNode(cpt->get_type().c_str())) {
            float fov = cpt->get_fov();

            ImGui::SliderFloat("FOV", &fov, 0.0, 180.0, "%.2f");

            cpt->set_fov(fov);

            ImGui::TreePop();
        }
    }

    template<>
    inline void component_menu<cpt::DirectionalLight>(World &world, const EntityID &entity) {
        const auto &cpt = world.get_component<cpt::DirectionalLight>(entity);
        if (ImGui::TreeNode(cpt->get_type().c_str())) {
            glm::vec3 color = cpt->get_color();
            float intensity = cpt->get_intensity();

            ImGui::ColorEdit3("Color", &color.r);
            ImGui::SliderFloat("Intensity", &intensity, 0.0, 1.0);

            cpt->set_color(color);
            cpt->set_intensity(intensity);

            ImGui::TreePop();
        }
    }

    template<>
    inline void component_menu<cpt::Environment>(World &world, const EntityID &entity) {
        const auto &cpt = world.get_component<cpt::Environment>(entity);
        if (ImGui::TreeNode(cpt->get_type().c_str())) {
            // Default behavior for any component type
            ImGui::Text("Environment");

            ImGui::TreePop();
        }
    }

    template<>
    inline void component_menu<cpt::FPSControl>(World &world, const EntityID &entity) {
        const auto &cpt = world.get_component<cpt::FPSControl>(entity);
        if (ImGui::TreeNode(cpt->get_type().c_str())) {
            // Default behavior for any component type
            ImGui::Text("FPSControl");

            ImGui::TreePop();
        }
    }

    template<>
    inline void component_menu<cpt::Orbiter>(World &world, const EntityID &entity) {
        const auto &cpt = world.get_component<cpt::Orbiter>(entity);
        if (ImGui::TreeNode(cpt->get_type().c_str())) {

            switch (cpt->get_current_mode()) {
                case cpt::ENTITY: {
                    const EntityID tracked_id = cpt->get_target_entity();

                    ImGui::Text("Current mode: ENTITY");

                    // Allow to change tracked entity at runtime
                    std::vector<EntityID> entities_id;
                    entities_id.reserve(world.get_entities().size());
                    std::string names = {};
                    for (const auto &[id, name]: world.get_entities()) {
                        entities_id.push_back(id);
                        names += name + '\0';
                    }
                    names += '\0';
                    static int current_item_idx = std::ranges::find(entities_id, tracked_id) - entities_id.begin();
                    ImGui::Combo("Tracked entity", &current_item_idx, names.c_str());
                    cpt->set_target(entities_id[current_item_idx]);
                } break;

                case cpt::WORLD_POINT: {
                    ImGui::Text("Current mode: WORLD POINT");
                    ImGui::Text("Looking at: (%.2f, %.2f, %.2f)", cpt->get_target_point().x, cpt->get_target_point().y,
                                cpt->get_target_point().z);

                    // Allow to change targeted point

                } break;

                default:
                    assert(false);
            }

            glm::vec3 offset = cpt->get_offset();
            float distance = cpt->get_distance();
            float hor_angle = cpt->get_hor_angle();
            float vert_angle = cpt->get_vert_angle();

            ImGui::InputFloat3("Offset", &offset.x);


            ImGui::SliderFloat("Distance", &distance, 0.0, 50.0, "%.2f", ImGuiSliderFlags_Logarithmic);
            cpt->set_distance(distance);

            ImGui::SliderFloat("Hor. Angle", &hor_angle, 0.0, 360.0, "%.2f");
            ImGui::SliderFloat("Ver. Angle", &vert_angle, -90.0, 90.0, "%.2f");

            cpt->set_offset(offset);
            cpt->set_distance(distance);
            cpt->set_hor_angle(hor_angle);
            cpt->set_vert_angle(vert_angle);

            ImGui::TreePop();
        }
    }

    template<>
    inline void component_menu<cpt::PointLight>(World &world, const EntityID &entity) {
        const auto &cpt = world.get_component<cpt::PointLight>(entity);
        if (ImGui::TreeNode(cpt->get_type().c_str())) {
            // Default behavior for any component type
            ImGui::Text("PointLight");
            ImGui::TreePop();
        }
    }

    template<>
    inline void component_menu<cpt::StaticModel>(World &world, const EntityID &entity) {
        const auto &cpt = world.get_component<cpt::StaticModel>(entity);
        if (ImGui::TreeNode(cpt->get_type().c_str())) {
            // Default behavior for any component type
            ImGui::Text("StaticModel");
            ImGui::TreePop();
        }
    }

    template<>
    inline void component_menu<cpt::Transform>(World &world, const EntityID &entity) {
        const auto &cpt = world.get_component<cpt::Transform>(entity);
        if (ImGui::TreeNode(cpt->get_type().c_str())) {
            glm::vec3 position = cpt->get_position();
            glm::quat rotation = cpt->get_rotation();
            glm::vec3 scale = cpt->get_scale();

            ImGui::InputFloat3("Position", &position.x);
            ImGui::InputFloat4("Rotation", &rotation.x);
            ImGui::InputFloat3("Scale", &scale.x);

            cpt->set_position(position);
            cpt->set_rotation(rotation);
            cpt->set_scale(scale);

            ImGui::TreePop();
        }
    }

    // Map to associate type indices with their corresponding menu functions
    static const std::map<std::type_index, ComponentMenuFunction> COMPONENT_FUNCTIONS = {
            {typeid(cpt::Camera3D), &component_menu<cpt::Camera3D>},
            {typeid(cpt::DirectionalLight), &component_menu<cpt::DirectionalLight>},
            {typeid(cpt::Environment), &component_menu<cpt::Environment>},
            {typeid(cpt::FPSControl), &component_menu<cpt::FPSControl>},
            {typeid(cpt::Orbiter), &component_menu<cpt::Orbiter>},
            {typeid(cpt::PointLight), &component_menu<cpt::PointLight>},
            {typeid(cpt::StaticModel), &component_menu<cpt::StaticModel>},
            {typeid(cpt::Transform), &component_menu<cpt::Transform>}};


    void render_component_window(World &world, bool *p_open = nullptr);

} // namespace wrld::gui
