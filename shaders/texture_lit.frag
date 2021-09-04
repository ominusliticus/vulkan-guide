#version 450

//shader input
layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_texture_coordinate;
//output write
layout (location = 0) out vec4 out_frag_color;

layout (set = 0, binding = 1) uniform  SceneData{   
    vec4 fog_color;			// w is for exponent
	vec4 fog_distances;		//x for min, y for max, zw unused.
	vec4 ambient_color;
	vec4 sunlight_direction; //w for sun power
	vec4 sunlight_color;
} sceneData;

layout (set = 2, binding = 0) uniform sampler2D texture1;

void main() 
{	
	vec3 color = texture(texture1, in_texture_coordinate).xyz;
	out_frag_color = vec4(color, 1.0f);
}