//
// Created by leo on 8/22/25.
//

#ifndef CUBEMAPTEXTURE_HPP
#define CUBEMAPTEXTURE_HPP
#include "glad/glad.h"


#include <string>
#include <vector>

namespace wrld::rsc {
    class CubemapTexture {
    public:
        /// Order of textures: +X, -X, +Y, -Y, +Z, -Z
        explicit CubemapTexture(const std::vector<std::string> &cubemap_paths);

        CubemapTexture(CubemapTexture &&other) noexcept;
        CubemapTexture &operator=(CubemapTexture &&other) noexcept;

        void use(unsigned unit = 0) const;

        ~CubemapTexture();

    private:
        GLuint gl_texture;
    };

} // namespace wrld::rsc

#endif // CUBEMAPTEXTURE_HPP
