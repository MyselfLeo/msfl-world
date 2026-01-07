//
// Created by leo on 9/12/25.
//

#pragma once

#include <wrld/resources/Resource.hpp>
#include <wrld/resources/Framebuffer.hpp>
#include "glad/glad.h"


namespace wrld::rsc {
    class DeferredFramebuffer : public Framebuffer {
    public:
        DeferredFramebuffer(std::string name, World &world);

        ~DeferredFramebuffer() override;

        [[nodiscard]] GLuint get_fbo() const override;

        DeferredFramebuffer &set_size(unsigned width, unsigned height) override;

        [[nodiscard]] unsigned get_width() const override;

        [[nodiscard]] unsigned get_height() const override;

        std::string get_type() const override { return "DeferredViewport"; }

        void use() const override;

        void recreate() override;

        [[nodiscard]] GLuint get_position_texture() const;

        [[nodiscard]] GLuint get_normal_texture() const;

        [[nodiscard]] GLuint get_diffuse_texture() const;

        [[nodiscard]] GLuint get_do_lighting_texture() const;

        [[nodiscard]] GLuint get_depth_texture() const;

    protected:
        GLuint fbo;

        GLuint position_texture;
        GLuint normal_texture;
        GLuint diffuse_texture; // diffuse + specular
        GLuint do_lighting_texture;
        GLuint depth_texture;

        unsigned width, height;
    };
} // namespace wrld::rsc
