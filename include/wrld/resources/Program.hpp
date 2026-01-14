//
// Created by leo on 8/8/25.
//

#pragma once

#include <wrld/resources/Material.hpp>

#include <string>

#include <glm/mat4x4.hpp>

namespace wrld::rsc {
    enum class ShaderType { Vertex, Fragment, Compute };

    std::string get_type_name(ShaderType type);

    unsigned int get_gl_type(ShaderType type);

    /// Represent a full program with different shaders.
    /// Each shader can either be defined from raw source or from a file.
    /// If both are defined for a given type, the raw source is used instead of the file.
    class Program final : public Resource {
    public:
        explicit Program(std::string name, World &world);

        /// Return the default Program (default graphic pipeline).
        Program &from_default();

        /// Set the source for the given shader type as a file.
        Program &shader_path(ShaderType shader_type, const std::string &path);

        /// set the source for the given shader type as raw string.
        Program &shader_source(ShaderType shader_type, const std::string &source);

        // /// Loads a shader file in which both shaders (vertex & fragment)
        // /// are defined (using #ifdef directives) to create the program.
        // Program &from_file(const std::string &combined_shader_path);
        //
        // /// Loads a shader separated in 2 files (one vertex, one fragment).
        // Program &from_file(const std::string &vertex_path,
        //                    const std::string &fragment_path);
        //
        // /// Loads a shader source in which both shaders (vertex & fragment)
        // /// are defined (using #ifdef directives) to create the program.
        // Program &from_source(const std::string &combined_shader_src);
        //
        // /// Loads a shader separated in 2 sources (one vertex, one fragment).
        // Program &from_source(const std::string &vertex_source,
        //                      const std::string &fragment_source);


        Program(Program &other) = delete;

        Program(Program &&other) = delete;

        Program &operator=(Program &other) = delete;

        Program &operator=(Program &&other) = delete;

        ~Program() override;

        /// Use this program in the GL context.
        /// May compile the program if not already done.
        void use();

        void set_uniform(const std::string &uniform, float value) const;

        void set_uniform(const std::string &uniform, int value) const;

        void set_uniform(const std::string &uniform, unsigned value) const;

        void set_uniform(const std::string &uniform, const glm::vec2 &value) const;

        void set_uniform(const std::string &uniform, const glm::vec3 &value) const;

        void set_uniform(const std::string &uniform, const glm::vec4 &value) const;

        void set_uniform(const std::string &uniform, const glm::mat4x4 &value) const;

        void set_uniform(const std::string &uniform, const glm::mat3x3 &value) const;

        void set_uniform(const std::string &uniform, const Rc<Material> &material) const;

        /// Reload & recompile all shaders of the program.
        /// If, for a given shader type, both a file and raw source is defined,
        /// the raw source is used.
        void reload();

        std::string get_type() const override { return "Program"; }

    private:
        std::unordered_map<ShaderType, std::string> shader_paths;
        std::unordered_map<ShaderType, std::string> shader_sources;
        // std::string vertex_shader_path;
        // std::string fragment_shader_path;

        // GLuint vertex_shader = 0;
        // GLuint fragment_shader = 0;

        unsigned int gl_program = 0;
        std::unordered_map<ShaderType, unsigned int> gl_shaders;

        static std::string read_file(const std::string &path);

        bool compiled_once = false;
        // void reload_from_file();
        // void reload_from_source(const std::string &vertex_src,
        //                         const std::string &fragment_src);

        /// Preprocess the GLSL source code to fit our needs.
        /// We add a #define with the expected type of the shader. This allows to
        /// combine the whole pipeline into one .glsl file.
        static std::string preprocess_source(const std::string &shader_source,
                                             ShaderType shader_type);

        static void compile_shader(unsigned int gl_shader, const std::string &shader_src);
    };
} // namespace wrld::rsc
