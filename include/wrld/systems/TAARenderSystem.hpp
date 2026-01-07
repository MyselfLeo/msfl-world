//
// Created by leo on 12/16/25.
//

#pragma once
#include <wrld/systems/DeferredRenderSystem.hpp>
#include <wrld/resources/TAAFramebuffer.hpp>

namespace wrld::sys {
    class TAARenderSystem final : public DeferredRenderSystem, public Singleton<TAARenderSystem> {
    public:
        TAARenderSystem();

        void init(World &world) override;

        /// Render all the cameras in the world.
        void render(World &world) override;

        LightCollection sample_lights(World &world) const;

        /// The image is computed as alpha * current_calculation + (1 - alpha) * history
        /// Alpha should be between 0 and 1
        [[nodiscard]] float get_alpha() const;

        /// The image is computed as alpha * current_calculation + (1 - alpha) * history
        /// Alpha should be between 0 and 1
        void set_alpha(float alpha);

    protected:
        /// Render the given camera.
        void render_camera(World &world, const cpt::Camera3D &camera,
                           const LightCollection &lights) override;

        /// Return data of a sample of PointLights in the world.
        /// The sample will have min(MAX_LIGHTS, point light count) / sample_pass_count
        /// lights at most.
        static std::vector<PointLightData> sample_point_lights(World &world);

        /// Return data of all DirectionalLights in the world.
        /// The sample will have min(MAX_LIGHTS, dir light count) / sample_pass_count
        /// lights at most.
        std::vector<DirectionalLightData> sample_directional_lights(World &world) const;

        void create_second_pass(World &world) override;

        void create_framebuffer(World &world) override;

        void update_framebuffer() override;

        void render_camera_second_pass(World &world, const cpt::Camera3D &camera,
                                       const LightCollection &lights) override;

        Rc<rsc::DeferredFramebuffer> get_framebuffer() const override;

        // /// Return count random indices in [0, max[.
        // static std::vector<size_t> sample_idx(size_t max, size_t count);

        GLfloat alpha = 0.5;

        static constexpr unsigned TAA_IMAGE_COUNT = 4;
        unsigned current_sample_pass = 0;

        Rc<rsc::TAAFramebuffer> g_framebuffer;

        Rc<rsc::Framebuffer> output_framebuffer;

        Rc<rsc::Program> deferred_second_pass;

        // todo: support multiple cameras
        // for that I need to change render_camera so it takes a Rc<Camera3D> that I can use
        // as a key for a unordered_map that store the previous transformation
        glm::mat4x4 previous_frame_transform; // Transform of the camera without model matrix)
        bool previous_frame_transform_set = false;

        // GLuint fbo;
        // GLuint history_texture;
        // GLuint depth_texture;
    };
}
