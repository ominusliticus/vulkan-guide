#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera
{
    float field_of_view;

    glm::vec3 position;
    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
    
    glm::mat4 GetCamera(glm::mat4& model);
};