//
// Created by leo on 1/6/26.
//

#pragma once

#include <wrld/resources/DeferredFramebuffer.hpp>

namespace wrld::rsc {
    class TAAFramebuffer : public DeferredFramebuffer {
    public:
        TAAFramebuffer(std::string name, World &world);

        [[nodiscard]] GLuint get_history_texture() const;

        void recreate() override;

        void use() const override;

    private:
        GLuint history_texture;
    };
}
