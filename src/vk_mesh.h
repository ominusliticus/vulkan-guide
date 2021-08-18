#pragma once

#include <vk_types.h>

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

struct VertexInputDescription
{
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    static VertexInputDescription GetVertexDescription();
};

struct Mesh
{
    std::vector<Vertex> vertices;
    AllocatedBuffer     vertex_buffer;

    // Read data from obj file
    bool LoadFromObj(const char* file_name);
};

struct MeshPushConstants
{
    glm::vec4 data;
    glm::mat4 render_matrix;
};
