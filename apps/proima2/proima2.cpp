//
// Created by leo on 11/18/25.
//

#include <wrld/App.hpp>
#include <wrld/Main.hpp>
using namespace wrld;

class ProIma2 final : public App {
public:
    ProIma2() = default;

    ~ProIma2() override = default;

    void init(World &world) override {
        // Create compute shader
        program = world.create_resource<rsc::Program>("compute");
        program->shader_path(rsc::ShaderType::Compute, "apps/proima2/filter.glsl");

        // genere quelques nombres
        std::vector<int> data(COUNT);
        for (unsigned i = 0; i < data.size(); i++)
            data[i] = rand() % 100;

        glGenBuffers(1, &m_gpu_buffer1);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_gpu_buffer1);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(int) * data.size(), data.data(),
                        0);

        glGenBuffers(1, &m_gpu_buffer2);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_gpu_buffer2);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(int) * data.size() + sizeof(int),
                        nullptr, 0);
        // glClearBufferData(m_gpu_buffer2, GL_R8, GL_RED, )

                glGenBuffers(1, &m_read_buffer);
        glBindBuffer(GL_COPY_READ_BUFFER, m_read_buffer);
        glBufferStorage(GL_COPY_READ_BUFFER, sizeof(int) * data.size(), nullptr,
                        GL_DYNAMIC_STORAGE_BIT);
    }

    void update(World &world, const double delta_time) override {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_gpu_buffer1);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_gpu_buffer2);

        int n = COUNT / 256; // COUNT threads, groupes de 256 threads...
        program->use();
        program->set_uniform("min", 40);

        glDispatchCompute(n, 1, 1);

        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

        // recupere le buffer resultat
        {
            glBindBuffer(GL_COPY_READ_BUFFER, m_read_buffer);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_gpu_buffer2);
            glCopyBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_COPY_READ_BUFFER, 0, 0,
                                sizeof(int) * COUNT);

            // recupere les valeurs
            std::vector<int> tmp(COUNT);
            glGetBufferSubData(GL_COPY_READ_BUFFER, 0, sizeof(int) * tmp.size(),
                               tmp.data());

            for (unsigned i = 0; i < tmp.size(); i++)
                printf("%d ", tmp[i]);
            printf("\n");
        }


        Main::exit();
    }

    void ui(World &world) override {}

    void exit(World &world) override {
        glDeleteBuffers(1, &m_gpu_buffer1);
        glDeleteBuffers(1, &m_gpu_buffer2);
        glDeleteBuffers(1, &m_read_buffer);
    }

private:
    unsigned COUNT = 1024;

    Rc<rsc::Program> program;

    GLuint m_gpu_buffer1;
    GLuint m_gpu_buffer2;
    GLuint m_read_buffer;
};

int main() {
    ProIma2 app{};
    Main::set_window_title("TP PROIMA");
    Main::set_renderer_type(RendererType::NoRenderer);
    Main::run(app, 1280, 900);
    return 0;
}
