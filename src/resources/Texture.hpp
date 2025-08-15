//
// Created by leo on 8/8/25.
//

#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include <string>

#include <glad/glad.h>

namespace wrld {
    class Texture {
    public:
        explicit Texture(const std::string &texture_path);

        void use() const;

        ~Texture();

    private:
        GLuint gl_texture;
    };
} // namespace wrld

#endif // TEXTURE_HPP
