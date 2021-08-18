#include <vk_mesh.h>

VertexInputDescription Vertex::GetVertexDescription()
{
    VertexInputDescription description;

    VkVertexInputBindingDescription main_binding{};
    main_binding.binding    = 0;
    main_binding.stride     = sizeof(Vertex);
    main_binding.inputRate  = VK_VERTEX_INPUT_RATE_VERTEX;

    description.bindings.push_back(main_binding);

    // positions will be stored at location 0
    VkVertexInputAttributeDescription position_attribute{};
    position_attribute.binding  = 0;
    position_attribute.location = 0;
    position_attribute.format   = VK_FORMAT_R32G32B32_SFLOAT;
    position_attribute.offset   = offsetof(Vertex, position);

    // positions will be stored at location 1
    VkVertexInputAttributeDescription normal_attribute{};
    normal_attribute.binding = 0;
    normal_attribute.location = 1;
    normal_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normal_attribute.offset = offsetof(Vertex, normal);

    // positions will be stored at location 1
    VkVertexInputAttributeDescription color_attribute{};
    color_attribute.binding = 0;
    color_attribute.location = 2;
    color_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    color_attribute.offset = offsetof(Vertex, color);

    description.attributes.push_back(position_attribute);
    description.attributes.push_back(normal_attribute);
    description.attributes.push_back(color_attribute);
    return description;
}