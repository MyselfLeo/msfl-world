//
// Created by leo on 12/7/25.
//

#pragma once

#include <string>

namespace wrld::shader::comp {
    /// Compute the visibility of each object (Renderable)
    /// based on the camera settings. The result (1 bool for each
    /// renderable) is placed in the visiblity_buffer.
    /// The test is the same as wrld::tools::is_visible.
    inline std::string VISIBILITY_CHECK = R"(
#version 460

/* ---------- Data structures & functions ------------ */
/*       See RenderSystem.hpp for informations.        */

// Axis-Aligned bounding box
struct AABoundingBox {
    vec4 lower; // We will consider .w to always be 1
    vec4 upper; // idem
};

// 8 corners of a box.
struct Box {
    vec4 corners[8];
};

struct StaticModelData {
    AABoundingBox bounding_box;
    uint mesh_start;
    uint mesh_count;
};

// Specifies that the given model should be drawn with the given transform.
struct Renderable {
    mat4 model_matrix;
    uint model_idx;
};

// Return as a box the axis-aligned bounding box,
// i.e. compute coordinates of each corner.
Box as_box(AABoundingBox aabb) {
    Box res;
    res.corners[0] = aabb.lower;
    res.corners[1] = vec4(aabb.upper.x, aabb.lower.y, aabb.lower.z, 1);
    res.corners[2] = vec4(aabb.lower.x, aabb.lower.y, aabb.upper.z, 1);
    res.corners[3] = vec4(aabb.upper.x, aabb.lower.y, aabb.upper.z, 1);
    res.corners[4] = vec4(aabb.lower.x, aabb.upper.y, aabb.lower.z, 1);
    res.corners[5] = vec4(aabb.upper.x, aabb.upper.y, aabb.lower.z, 1);
    res.corners[6] = vec4(aabb.lower.x, aabb.upper.y, aabb.upper.z, 1);
    res.corners[7] = aabb.upper;
    return res;
}

// Apply the transform matrix to each corner of the box.
// Divides by w to return normalized coordinates.
void transform(inout Box box, mat4 transform_matrix) {
    for (int i = 0; i < 8; i++) {
        box.corners[i] = transform_matrix * box.corners[i];
        box.corners[i] = box.corners[i] / box.corners[i].w;
    }
}

// Apply the transform matrix to the given aabb.
// It will no longer be axis-aligned so we return its
// corners instead.
Box transform(AABoundingBox aabb, mat4 transform_matrix) {
    Box res = as_box(aabb);
    transform(res, transform_matrix);
    return res;
}


// Returns false if all corners of 'other' are all outside
// the 'aligned' in regard to one particular plane of 'aligned'.
// This is half a test for frustum culling
bool may_collide(AABoundingBox aligned, Box other) {
    // Lower bounds
    for (int axis = 0; axis < 3; axis++) {
        bool all = true;
        for (int i = 0; i < 8; i++) {
            if (other.corners[i][axis] > aligned.lower[axis]) {
                all = false;
            }
        }
        if (all) {
            return false;
        }
    }

    // Upper bounds
    for (int axis = 0; axis < 3; axis++) {
        bool all = true;
        for (int i = 0; i < 8; i++) {
            if (other.corners[i][axis] < aligned.upper[axis]) {
                all = false;
            }
        }
        if (all) {
            return false;
        }
    }

    return true;
}

/* ---------- IO ------------ */

// Input data
layout (std430, binding = 0) readonly buffer model_data_buffer {
    StaticModelData model_data[];
};
layout (std430, binding = 1) readonly buffer renderable_buffer {
    Renderable renderables[];
};

// Output data
layout (std430, binding = 2) writeonly buffer visibility_buffer {
    bool visibility[];
};

// Camera settings
// fixme: Move to uniform buffer ?
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

/* ---------- Main function ------------ */

const AABoundingBox proj_frustum = AABoundingBox(vec4(-1, -1, -1, 1), vec4(1, 1, 1, 1));

layout (local_size_x = 256) in; // Test up to 256 renderables at once
void main() {
    uint id = gl_GlobalInvocationID.x;

    if (id < renderables.length()) {
        StaticModelData model = model_data[renderables[id].model_idx];
        mat4 transform_matrix = proj_matrix * view_matrix * renderables[id].model_matrix;

        AABoundingBox local_object = model.bounding_box;
        Box proj_object = transform(local_object, transform_matrix);
        Box local_frustum = transform(proj_frustum, inverse(transform_matrix));

        visibility[id] = may_collide(proj_frustum, proj_object) && may_collide(local_object, local_frustum);
    }
}
)";
}
