//
// Created by leo on 9/18/25.
//

#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP
#include <type_traits>

namespace wrld {
    class Resource;
    class Component;

    /// Concept of a Resource: it must inherit the rsc::Resource class
    template<class T>
    concept ResourceConcept = std::is_base_of_v<Resource, T>;

    /// Concept of a Component: it must inherit the cpt::Component class
    template<class T>
    concept ComponentConcept = std::is_base_of_v<Component, T>;

} // namespace wrld

#endif // CONCEPTS_HPP
