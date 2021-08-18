
// 3rd party
#include <tiny_obj_loader.h>

// STL
#include <iostream>
#include <string>

// Mine
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

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

bool Mesh::LoadFromObj(const char* file_name)
{
    tinyobj::attrib_t attribute;                // Contains vertex array from file
    std::vector<tinyobj::shape_t> shapes;       // info on separate object in file
    std::vector<tinyobj::material_t> materials; 

    // error handling vars
    std::string warning;
    std::string error;

    // load file
    tinyobj::LoadObj(&attribute, &shapes, &materials, &warning, &error, file_name, nullptr);
    if (!warning.empty()) std::cout << "WARNING: " << warning << std::endl;
    if (!error.empty())
    {
        std::cout << "ERROR: " << error << std::endl;
        return false;
    }

    // loop over shapes
    for (size_t ishape = 0; ishape < shapes.size(); ishape++)
    {
        size_t index_offset = 0;
        // loop over face of polygon
        for (size_t iface = 0; iface < shapes[ishape].mesh.num_face_vertices.size(); iface++)
        {
            int vertices_in_triangle = 3;
            // loop over vertices for face
            for (size_t ivertex = 0; ivertex < vertices_in_triangle; ivertex++)
            {
                // access vertex
                tinyobj::index_t idx = shapes[ishape].mesh.indices[index_offset + ivertex];

                // vertex position
                tinyobj::real_t vx = attribute.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attribute.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attribute.vertices[3 * idx.vertex_index + 2];

                // vertex normal
                tinyobj::real_t nx = attribute.normals[3 * idx.normal_index+ 0];
                tinyobj::real_t ny = attribute.normals[3 * idx.normal_index+ 1];
                tinyobj::real_t nz = attribute.normals[3 * idx.normal_index+ 2];

                // Create vertex with above information
                Vertex new_vertex;
                new_vertex.position.x = vx;     new_vertex.normal.x = nx;
                new_vertex.position.y = vy;     new_vertex.normal.y = ny;
                new_vertex.position.z = vz;     new_vertex.normal.z = nz;

                new_vertex.color = new_vertex.normal;
                vertices.push_back(new_vertex);
            } // end for : ivertex
            index_offset += vertices_in_triangle;
        } // end for : iface
    } // end for : ishape
    
    return true;
}