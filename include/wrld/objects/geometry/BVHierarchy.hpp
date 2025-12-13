//
// Created by leo on 11/19/25.
//

#pragma once
#include <iostream>
#include <memory>
#include <span>
#include <vector>

#include <wrld/objects/geometry/AABoundingBox.hpp>
#include <wrld/concepts.hpp>

namespace wrld::obj {
    struct BVHLeaf {
        explicit BVHLeaf(const std::span<unsigned> &element_ids);

        std::span<unsigned> element_ids;
    };

    template<GeoRequestableConcept O>
    struct BVHNode {
        explicit BVHNode(std::span<unsigned> element_ids, const std::vector<O> &elements,
                         const unsigned group_size) {
            // Prevent spawning too much OpenMP tasks during BVH construction
            constexpr size_t task_spawn_threshold = 10240;

            // Based on the size of element_ids, we either create a Leaf or split
            // into 2 nodes

            // Compute bounding box. It will be used later if it is a node
            bounding_box = {};
            for (const auto i: element_ids) {
                bounding_box = bounding_box + elements[i].get_bounding_box();
            }

            // Create a leaf
            if (element_ids.size() < group_size) {
                leaf = new BVHLeaf(element_ids);
            }

            // Split along the largest axis and create 2 nodes {
            else {
                // Find largest axis
                int axis = 0;
                const auto size = bounding_box.size();
                for (int i = 1; i < 3; i++) {
                    if (size[i] > size[axis]) {
                        axis = i;
                    }
                }

                // Sort elements along the axis
                const auto compare = [&](const unsigned id1, const unsigned id2) {
                    return elements[id1].center()[axis] < elements[id2].center()[axis];
                };
                std::ranges::sort(element_ids, compare);

                // Split in the middle & generate subnodes
                const int left_size = element_ids.size() / 2;
                const int right_size = element_ids.size() - left_size;

                std::span left_span = element_ids.subspan(0, left_size);
                std::span right_span = element_ids.subspan(left_size, right_size);

                if (element_ids.size() > task_spawn_threshold) {
                    #pragma omp task firstprivate(left_span, group_size) shared(elements)
                    left_child = new BVHNode(left_span, elements, group_size);

                    #pragma omp task firstprivate(right_span, group_size) shared(elements)
                    right_child = new BVHNode(right_span, elements, group_size);
                } else {
                    left_child = new BVHNode(left_span, elements, group_size);
                    right_child = new BVHNode(right_span, elements, group_size);
                }
            }
        }

        ~BVHNode() {
            delete left_child;
            delete right_child;
            delete leaf;
        }

        [[nodiscard]] AABoundingBox get_bounding_box() const { return bounding_box; }

        [[nodiscard]] bool include(const glm::vec3 &point,
                                   const std::vector<O> &elements) const {
            if (!get_bounding_box().include(point))
                return false;

            if (leaf != nullptr) {
                for (const auto i: leaf->element_ids) {
                    if (elements[i].include(point))
                        return true;
                }
                return false;
            }

            return left_child->include(point, elements) ||
                   right_child->include(point, elements);
        }

        [[nodiscard]] std::optional<double>
        intersect(const Ray &ray, const std::vector<O> &elements) const {
            if (!get_bounding_box().intersect(ray))
                return std::nullopt;

            if (leaf != nullptr) {
                std::optional<double> res;

                // Check all elements for the smallest distance
                for (const auto i: leaf->element_ids) {
                    auto intersect = elements[i].intersect(ray);
                    if (intersect.has_value()) {
                        if (!res.has_value() || intersect.value() < res.value()) {
                            res = intersect;
                        }
                    }
                }

                return res;
            }

            auto res_left = left_child->intersect(ray, elements);
            auto res_right = right_child->intersect(ray, elements);

            if (res_left.has_value() && res_right.has_value()) {
                return std::min(res_left.value(), res_right.value());
            }
            if (res_left.has_value())
                return res_left.value();
            if (res_right.has_value())
                return res_right.value();
            return std::nullopt;
        }

        [[nodiscard]] glm::vec3 center() const { return bounding_box.center(); }

        // We test on the BB first
        AABoundingBox bounding_box;

        // Either leaf OR left_child & right_child null, not both
        BVHLeaf *leaf = nullptr;
        BVHNode *left_child = nullptr;
        BVHNode *right_child = nullptr;
    };

    /// Binary tree for fast geometrical requests.
    /// Must be built in the use space, as it can't really be
    /// transformed from one space to another.
    template<GeoRequestableConcept O>
    class BVHierarchy : public GeoRequestable {
    public:
        /// Empty BVH. Unusable
        BVHierarchy() : root_node(nullptr) {
        }

        /// Build a BVHierarchy that will allow fast geo requests over the specified
        /// elements.
        /// group_size determines the maximum amount of elements in a leaf. If
        /// group_size > 1, leaves of the BVH will contain multiple elements and will
        /// have to check them in a loop. The value to give to group_size depends on
        /// the nature of the elements.
        explicit BVHierarchy(const std::vector<O> &_elements, unsigned group_size) {
            if (group_size == 0)
                throw std::runtime_error("BVH Group size cannot be 0.");

            // Copy the elements into our managed vector
            elements = std::make_unique<std::vector<O> >();
            *elements = _elements;

            // We'll sorts the ids instead of the elements, it's easier
            indices = std::make_unique<std::vector<unsigned> >();
            indices->reserve(elements->size());
            for (int i = 0; i < elements->size(); i++)
                indices->push_back(i);

            std::span ids_span(*indices);

            #pragma omp parallel
            {
                #pragma omp single
                {
                    root_node = new BVHNode(ids_span, *elements, group_size);
                }
            }
        }

        BVHierarchy(BVHierarchy &&other) noexcept : root_node(other.root_node), group_size(other.group_size),
                                                    elements(std::move(other.elements)),
                                                    indices(std::move(other.indices)) {
            other.root_node = nullptr;
            other.elements = nullptr;
            other.indices = nullptr;
        }

        BVHierarchy &operator=(BVHierarchy &&other) noexcept {
            this->root_node = other.root_node;
            this->elements = std::move(other.elements);
            this->indices = std::move(other.indices);
            this->group_size = other.group_size;

            other.root_node = nullptr;
            other.elements = nullptr;
            other.indices = nullptr;

            return *this;
        }

        BVHierarchy(BVHierarchy &other) = delete;

        ~BVHierarchy() override { delete root_node; }

        /// Return true if the BVH is built and ready for geometrical requests
        [[nodiscard]] bool built() const { return root_node != nullptr; }

        /// Return the bounding box of the BVH.
        /// Throws if the BVH is not built.
        [[nodiscard]] AABoundingBox get_bounding_box() const override {
            if (!built())
                throw std::runtime_error("The BVHierarchy has not been built !");

            return root_node->get_bounding_box();
        }

        /// Performs an intersection test between the content of the BVH and the ray.
        /// Throws if the BVH is not built.
        [[nodiscard]] std::optional<double> intersect(const Ray &ray) const override {
            if (!built())
                throw std::runtime_error("The BVHierarchy has not been built !");

            return root_node->intersect(ray, *elements);
        }

        /// Performs an inclusion test between the content of the BVH and the point.
        /// Throws if the BVH is not built.
        [[nodiscard]] bool include(const glm::vec3 &point) const override {
            if (!built())
                throw std::runtime_error("The BVHierarchy has not been built !");

            return root_node->include(point, *elements);
        }

        /// Returns the center of the BVH.
        /// Throws if the BVH is not built.
        [[nodiscard]] glm::vec3 center() const override {
            return get_bounding_box().center();
        }

    private
    :
        BVHNode<O> *root_node = nullptr;
        unsigned group_size = 1;

        std::unique_ptr<std::vector<O> > elements;
        std::unique_ptr<std::vector<unsigned> > indices;
    };
} // namespace wrld::obj
