//
// Created by leo on 10/2/25.
//

#include <wrld/components/CameraShader.hpp>

namespace wrld::cpt {
    CameraShader::CameraShader(const EntityID entity_id, World &world,
                               const std::shared_ptr<rsc::WindowFramebuffer> &viewport,
                               const Rc<rsc::Program> &program) :
        Component(entity_id, world), framebuffer(viewport), program(program) {}

    Rc<rsc::Program> CameraShader::get_program() const { return program; }

    void CameraShader::set_program(const Rc<rsc::Program> &program) { this->program = program; }

} // namespace wrld::cpt
