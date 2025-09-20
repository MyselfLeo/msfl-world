//
// Created by leo on 9/6/25.
//

#include "Resource.hpp"

#include "Material.hpp"
#include "Rc.hpp"
#include "World.hpp"

#include <utility>

namespace wrld {
    Resource::Resource(std::string name, World &world /*, Rc<Resource> *rc*/) :
        name(std::move(name)), world(world) /*, rc(rc)*/ {}

    std::string Resource::get_name() const { return name; }

    bool Resource::has_resource(const std::string &unique_name) const {
        return attached_resources.contains(unique_name);
    }

    void Resource::detach_resource(const std::string &unique_name) {
        if (!attached_resources.contains(unique_name))
            return;

        attached_resources[unique_name].detach_resource_user(std::type_index(typeid(*this)), get_name());
    }

    // void Resource::detach_resource(const std::string &unique_name) {
    //     if (!attached_resources.contains(unique_name))
    //         return;
    //
    //     attached_resources[unique_name].detach_resource_user(get_name());
    // }

    // void Resource::attach_resource(const std::string &unique_name, const std::shared_ptr<Rc<Resource>> &resource) {
    //     if (attached_resources.contains(unique_name)) {
    //         attached_resources[unique_name]->detach_resource_user(this);
    //     }
    //
    //     resource->attach_resource_user(shared_from_this());
    //     attached_resources[unique_name] = resource;
    // }

} // namespace wrld
