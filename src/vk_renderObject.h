#pragma once

#include <vk_types.h>
#include <vk_mesh.h>
#include <vk_material.h>

#include <glm/glm.hpp>

struct RenderObject
{
    Mesh*       mesh;
    Material*   material;
    glm::mat4   transform_matrix;
};