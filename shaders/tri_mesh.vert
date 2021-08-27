#version 460 

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_color;

layout (location = 0) out vec3 out_color;

struct ObjectData
{
	mat4 model;
};

layout (set = 0, binding = 0) uniform CameraBuffer
{
	mat4 view_matrix;
	mat4 projection_matrix;
	mat4 view_projection;
} CameraData;

layout (std140, set = 1, binding = 0) readonly buffer ObjectBuffer
{
	ObjectData objects[];
} objectBuffer;

// push constants
layout( push_constant ) uniform constants
{
	vec4 data;
	mat4 render_matrix;
} PushConstants;

void main()
{
	mat4 model_matrix		= objectBuffer.objects[gl_BaseInstance].model;
	mat4 transform_matrix	= (CameraData.view_projection * model_matrix);
	gl_Position				= transform_matrix * vec4(vertex_position, 1.0f);
	out_color				= vertex_color;
}