

#include <Camera.h>

void Camera::ProcessInput(CameraDirection dir, float speed)
{
    if (dir == CameraDirection::FORWARD)
        position += front * speed;
    if (dir == CameraDirection::BACKWARD)
        position += -front * speed;
    if (dir == CameraDirection::LEFT)
        position += -right * speed;
    if (dir == CameraDirection::RIGHT)
        position += right * speed;
}

void Camera::UpdateRotation(float x, float y, VkExtent2D extent)
{
    // Add some logic that makes camera turn continuously when off screen 
    // but otherwise only moves when a change in mouse position is detected.
    float sensitivity = 0.001;
    glm::vec2 origin = { 0.5 * extent.width, 0.5 * extent.height };
    glm::vec2 pos = glm::vec2{ x, y } - origin;

    if (last_x != x || last_y != y)
    {

        yaw     += sensitivity * pos.x;
        pitch   += sensitivity * pos.y;

        if (pitch > 90) pitch = 90;
        if (pitch < -90) pitch = -90;

        last_x = x;
        last_y = y;
    }
    if (x <= 0) yaw += -0.5;
    if (x >= extent.width-1) yaw += 0.5;
    if (y <= 0)
    {
        pitch += -0.5;
        if (pitch < -90) pitch = -90;
    }
    if (y >= extent.height-1)
    {
        pitch += 0.5;
        if (pitch > 90) pitch = 90;
    }
    UpdateVectors();
}

void Camera::UpdateVectors()
{
    glm::vec3 temp{};
    temp.x  = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    temp.y  = -std::sin(glm::radians(pitch));
    temp.z  = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    front   = glm::normalize(temp);

    right   = glm::normalize(glm::cross(front, world_up));
    up      = glm::normalize(glm::cross(right, front));
}


glm::mat4 Camera::GetCamera(glm::mat4& model)
{
    view_matrix = glm::lookAt(position, position + front, up);
    return projection_matrix * view_matrix * model;
}