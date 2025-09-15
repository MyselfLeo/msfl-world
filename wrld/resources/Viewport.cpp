//
// Created by leo on 9/12/25.
//

#include "Viewport.hpp"

namespace wrld::rsc {
    Viewport::Viewport(std::string name, World &world) : Resource(std::move(name), world) {}

} // namespace wrld::rsc
