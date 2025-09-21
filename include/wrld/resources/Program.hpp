//
// Created by leo on 8/8/25.
//

#ifndef PROGRAM_HPP
#define PROGRAM_HPP
#include <wrld/resources/Material.hpp>

#include <string>

#include <glad/glad.h>

#include "glm/mat4x4.hpp"

namespace wrld::rsc {
    enum ShaderType { VERTEX_SHADER, FRAGMENT_SHADER };

    std::string get_type_name(ShaderType type);

    /// Combines a vertex and a fragment shader
    class Program final : public Resource {
    public:
        explicit Program(std::string name, World &world /*, Rc<Resource> *rc*/);

        /// Loads a shader file in which both shaders (vertex & fragment)
        /// are defined (using #ifdef directives) to create the program.
        Program &from_file(const std::string &combined_shader_path);

        /// Loads a shader separated in 2 files (one vertex, one fragment).
        Program &from_file(const std::string &vertex_path, const std::string &fragment_path);

        /// Loads a shader source in which both shaders (vertex & fragment)
        /// are defined (using #ifdef directives) to create the program.
        Program &from_source(const std::string &combined_shader_src);

        /// Loads a shader separated in 2 sources (one vertex, one fragment).
        Program &from_source(const std::string &vertex_source, const std::string &fragment_source);


        Program(Program &other) = delete;
        Program(Program &&other) = delete;
        Program &operator=(Program &other) = delete;
        Program &operator=(Program &&other) = delete;

        ~Program() override;

        /// Use this program in the GL context.
        void use() const;

        void set_uniform(const std::string &uniform, float value) const;

        void set_uniform(const std::string &uniform, int value) const;

        void set_uniform(const std::string &uniform, unsigned value) const;

        void set_uniform(const std::string &uniform, const glm::vec3 &value) const;

        void set_uniform(const std::string &uniform, const glm::vec4 &value) const;

        void set_uniform(const std::string &uniform, const glm::mat4x4 &value) const;

        void set_uniform(const std::string &uniform, const glm::mat3x3 &value) const;

        void set_uniform(const std::string &uniform, const Material &material) const;

        void reload() const;

        std::string get_type() const override { return "Program"; }

    private:
        std::string vertex_shader_path = "wrld/shaders/vertex/default.glsl";
        std::string fragment_shader_path = "wrld/shaders/fragment/default.glsl";

        GLuint vertex_shader = 0;
        GLuint fragment_shader = 0;
        GLuint gl_program = 0;

        static std::string read_file(const std::string &path);

        bool compiled_once = false;
        void reload_from_file();
        void reload_from_source(const std::string &vertex_src, const std::string &fragment_src);

        /// Preprocess the GLSL source code to fit our needs.
        /// We add a #define with the expected type of the shader. This allows to
        /// combine the whole pipeline into one .glsl file.
        static std::string preprocess_source(const std::string &shader_source, ShaderType shader_type);

        static void compile_shader(GLuint gl_shader, const std::string &shader_src, ShaderType type);
    };
} // namespace wrld::rsc

#endif // PROGRAM_HPP
