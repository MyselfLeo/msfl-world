//
// Created by leo on 9/12/25.
//

#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP
#include "Resource.hpp"
#include "glad/glad.h"

namespace wrld::rsc {
    class Viewport : public Resource {
    public:
        Viewport(std::string name, World &world);

        ~Viewport() override;

        [[nodiscard]] virtual GLenum get_fbo() const = 0;
        [[nodiscard]] virtual unsigned get_width() const = 0;
        [[nodiscard]] virtual unsigned get_height() const = 0;

        std::string get_type() override { return "Viewport"; }
    };

} // namespace wrld::rsc

#endif // VIEWPORT_HPP
