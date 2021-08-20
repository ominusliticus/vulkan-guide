#version 450 

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_color;

layout (location = 0) out vec3 out_color;

// push constants
layout( push_constant ) uniform constants
{
	vec4 data;
	mat4 render_matrix;
} PushConstants;

void main()
{
	gl_Position = PushConstants.render_matrix * vec4(vertex_position, 1.0f);
	out_color = vertex_color;
}