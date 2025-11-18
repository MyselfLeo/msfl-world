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

    void init(World &world) override {}

    void update(World &world, const double delta_time) override { Main::exit(); }

    void ui(World &world) override {}

    void exit(World &world) override {}

private:
};

int main() {
    ProIma2 app{};
    Main::set_window_title("TP PROIMA");
    Main::set_renderer_type(RendererType::NoRenderer);
    Main::run(app, 1280, 900);
    return 0;
}
