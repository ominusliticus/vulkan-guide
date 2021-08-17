#version 450

void main()
{
	// const array of vertices for triangle
	const vec3 positions[3] = vec3[3](
		vec3(1.0f, 1.0f, 0.0f),
		vec3(-1.0f, 1.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f)
	);

	// output vertices
	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
}