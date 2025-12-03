//
// Created by leo on 9/18/25.
//
#pragma once

#include <type_traits>
#include <wrld/objects/geometry/GeoRequestable.hpp>

namespace wrld {
    class Resource;
    class Component;

    // Helper trait to check if a type is a specialization of std::tuple
    template<typename T>
    struct is_tuple : std::false_type {};
    template<typename... Ts>
    struct is_tuple<std::tuple<Ts...>> : std::true_type {};
    template<typename T>
    constexpr bool is_tuple_v = is_tuple<T>::value;


    /// Concept of a Resource: it must inherit the rsc::Resource class
    template<class T>
    concept ResourceConcept = std::is_base_of_v<Resource, T>;

    /// Concept of a Component: it must inherit the cpt::Component class
    template<class T>
    concept ComponentConcept = requires
    {
        // Should inherit Component
        std::is_base_of_v<Component, T>;

        // Should defined a sub-type requirements
        typename T::required_components;
        requires is_tuple_v<typename T::required_components>;
    };

    template<class T>
    concept GeoRequestableConcept = std::is_base_of_v<obj::GeoRequestable, T>;

} // namespace wrld
