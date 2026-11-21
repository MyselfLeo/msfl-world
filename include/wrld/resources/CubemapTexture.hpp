//
// Created by leo on 8/22/25.
//

#pragma once

#define GLEW_NO_GLU

#include <wrld/resources/Resource.hpp>
#include <GL/glew.h>


#include <string>
#include <vector>

namespace wrld::rsc {
    class CubemapTexture final : public Resource {
    public:
        explicit CubemapTexture(std::string name, World &world /*, Rc<Resource> *rc*/);

        /// Order of textures: +X, -X, +Y, -Y, +Z, -Z
        CubemapTexture &from_texture(const std::vector<std::string> &cubemap_paths);

        CubemapTexture(CubemapTexture &other) = delete;

        CubemapTexture(CubemapTexture &&other) = delete;

        CubemapTexture &operator=(CubemapTexture &other) = delete;

        CubemapTexture &operator=(CubemapTexture &&other) = delete;

        void use(unsigned unit = 0) const;

        ~CubemapTexture() override;

        std::string get_type() const override { return "CubemapTexture"; }

    private:
        GLuint gl_texture;
    };
} // namespace wrld::rsc
