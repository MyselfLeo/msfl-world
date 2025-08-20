//
// Created by leo on 8/20/25.
//

#ifndef DEBUGMOVE_HPP
#define DEBUGMOVE_HPP
#include "Component.hpp"

namespace wrld::cpt {

    class DebugMove final : public Component {
    public:
        DebugMove(EntityID entity_id, World &world);

    private:
    };

} // namespace wrld::cpt

#endif // DEBUGMOVE_HPP
