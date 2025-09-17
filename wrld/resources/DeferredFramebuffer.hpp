//
// Created by leo on 9/12/25.
//

#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP
#include "Resource.hpp"
#include "glad/glad.h"

#include <vector>

namespace wrld::rsc {
    class DeferredFramebuffer : public Resource {
    public:
        DeferredFramebuffer(std::string name, World &world);

        ~DeferredFramebuffer() override;

        [[nodiscard]] GLuint get_fbo() const;

        DeferredFramebuffer &set_size(unsigned width, unsigned height);

        [[nodiscard]] unsigned get_width() const;
        [[nodiscard]] unsigned get_height() const;

        std::string get_type() override { return "DeferredViewport"; }

        void use() const;

        void recreate();

        [[nodiscard]] GLuint get_position_texture() const;
        [[nodiscard]] GLuint get_normal_texture() const;
        [[nodiscard]] GLuint get_diffuse_texture() const;
        [[nodiscard]] GLuint get_depth_texture() const;

    private:
        friend class DeferredRendererSystem;

        GLuint fbo;

        GLuint position_texture;
        GLuint normal_texture;
        GLuint diffuse_texture;
        GLuint depth_texture;
        // GLuint specular_texture;

        unsigned width, height;
    };

} // namespace wrld::rsc

#endif // VIEWPORT_HPP
