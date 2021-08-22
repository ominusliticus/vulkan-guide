

#include <Camera.h>

glm::mat4 Camera::GetCamera(glm::mat4& model)
{
    view_matrix = glm::translate(glm::mat4{ 1.0f }, position);
    return projection_matrix * view_matrix * model;
}