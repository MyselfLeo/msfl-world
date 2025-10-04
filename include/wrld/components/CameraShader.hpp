//
// Created by leo on 10/2/25.
//

#pragma once
#include <wrld/components/Component.hpp>
#include <wrld/resources/Program.hpp>
#include <wrld/resources/WindowFramebuffer.hpp>

namespace wrld::cpt {

    class CameraShader final : public Component {
    public:
        explicit CameraShader(EntityID entity_id, World &world, const std::shared_ptr<rsc::WindowFramebuffer> &viewport,
                              const Rc<rsc::Program> &program);

        [[nodiscard]] Rc<rsc::Program> get_program() const;

        void set_program(const Rc<rsc::Program> &program);

    private:
        std::shared_ptr<rsc::WindowFramebuffer> framebuffer;
        Rc<rsc::Program> program;
    };


} // namespace wrld::cpt
