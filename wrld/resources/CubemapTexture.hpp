//
// Created by leo on 8/22/25.
//

#ifndef CUBEMAPTEXTURE_HPP
#define CUBEMAPTEXTURE_HPP
#include "Resource.hpp"
#include "glad/glad.h"


#include <string>
#include <vector>

namespace wrld::rsc {
    class CubemapTexture final : public Resource {
    public:
        /// Order of textures: +X, -X, +Y, -Y, +Z, -Z
        explicit CubemapTexture(std::string name, World &world /*, Rc<Resource> *rc*/);

        CubemapTexture &set_texture(const std::vector<std::string> &cubemap_paths);

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

#endif // CUBEMAPTEXTURE_HPP
