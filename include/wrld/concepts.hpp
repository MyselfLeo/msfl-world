//
// Created by leo on 9/18/25.
//
#pragma once

#include <type_traits>
#include <wrld/objects/geometry/GeoRequestable.hpp>

namespace wrld {
    class Resource;
    class Component;

    /// Concept of a Resource: it must inherit the rsc::Resource class
    template<class T>
    concept ResourceConcept = std::is_base_of_v<Resource, T>;

    /// Concept of a Component: it must inherit the cpt::Component class
    template<class T>
    concept ComponentConcept = std::is_base_of_v<Component, T>;

    template<class T>
    concept GeoRequestableConcept = std::is_base_of_v<obj::GeoRequestable, T>;

} // namespace wrld
