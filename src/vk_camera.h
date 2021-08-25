#pragma once


#include <vk_types.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraDirection
{
    FORWARD = 0, BACKWARD, LEFT, RIGHT
};

class Camera
{
public:
    float field_of_view;
    float pitch;
    float yaw;

    glm::vec3 up;                           // y direction is camera frame
    glm::vec3 position;                     // position of camera
    glm::vec3 front;                        // the direction the camera is pointing
    glm::vec3 right;                        // used to calculate front up
    glm::vec3 world_up{ 0.0, 1.0, 0.0 };   // used to calculate right vector
    glm::mat4 view_matrix;                  // transforms world object onto 2d screen
    glm::mat4 projection_matrix;

    float last_x;
    float last_y;
    
    void        ProcessInput(CameraDirection dir, float speed);
    void        UpdateRotation(float x, float y, VkExtent2D extent);
    glm::mat4   GetCamera(glm::mat4& model);

private:
    void UpdateVectors();
};

// will hold the descriptor set for Camera class
struct GPUCameraData
{
    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
    glm::mat4 view_projection;      // Product of view and projection matrix
};