//
// Created by leo on 11/27/25.
//

#pragma once

#include <string>

namespace wrld::shader {
    inline std::string FRUSTUM_CULLING = R"(
#version 430

/* Compute the visibility of each object based on the camera
 * settings.
 * The test is the same as wrld::tools::is_visible.
 */

/* ---------- Data structures & functions ------------ */

// Axis-Aligned bounding box
struct AABoundingBox {
    vec4 lower; // We will consider .w to always be 1
    vec4 upper; // idem
};

// 8 corners of a box.
struct Box {
    vec4 corners[8];
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

// Local -> World transform matrix of each object
layout(std430, binding = 0) readonly buffer model_matrices_buffer {
    mat4 matrices[];
};

// Local space AABB of each object
layout(std430, binding = 1) readonly buffer aabb_buffer {
    AABoundingBox bounding_boxes[];
};

// Result buffer
layout(std430, binding = 2) writeonly buffer result_buffer {
    bool visible[];
};

// Camera settings
uniform mat4 view_matrix;
uniform mat4 proj_matrix;


/* ---------- Main function ------------ */
const AABoundingBox proj_frustum = AABoundingBox(vec4(-1, -1, -1, 1), vec4(1, 1, 1, 1));

layout(local_size_x = 256) in; // Test up to 256 objects at once
void main() {
    uint id = gl_GlobalInvocationID.x;

    if (id < matrices.length()) {
        mat4 transform_matrix = proj_matrix * view_matrix * matrices[id];

        AABoundingBox local_object = bounding_boxes[id];
        Box proj_object = transform(local_object, transform_matrix);
        Box local_frustum = transform(proj_frustum, inverse(transform_matrix));

        // First pass : in projective space
        if (!may_collide(proj_frustum, proj_object)) {
            visible[id] = false;
        }
        // Second pass : in local space
        else if (!may_collide(local_object, local_frustum)) {
            visible[id] = false;
        } else {
            visible[id] = true;
        }
    }
}
)";

}
