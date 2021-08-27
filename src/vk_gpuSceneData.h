#pragma once

#include <glm/glm.hpp>

struct GPUSceneData
{
	glm::vec4 fog_color;
	glm::vec4 fog_distance;
	glm::vec4 ambient_color;
	glm::vec4 sunlight_direction;
	glm::vec4 sunlight_color;
};

struct GPUObjectData
{
	glm::mat4 model_matrix;
};