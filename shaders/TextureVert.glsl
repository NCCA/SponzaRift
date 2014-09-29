#version 400 core
#pragma optionNV(unroll all)
// first attribute the vertex values from our VAO
layout (location =0) in vec3 inVert;
// second attribute the UV values from our VAO
layout (location =1) in vec2 inUV;
// third attribute the  normals values from our VAO
layout (location =2) in vec3 inNormal;
// forth attribute the Tangents values from our VAO
layout (location =3) in vec3 inTangent;
// fith attribute the binormal values from our VAO
layout (location =4) in vec3 inBinormal;

// transform matrix values
uniform mat4 MVP;
uniform mat3 normalMatrix;
uniform mat4 MV;
struct LightInfo
{
	// Light position in eye coords.
	vec4 position;
	// Ambient light intensity
	vec3 La;
	// Diffuse light intensity
	vec3 Ld;
	// Specular light intensity
	vec3 Ls;
};
uniform LightInfo light;
// we use this to pass the UV values to the frag shader
out vec2 vertUV;
out vec3 position;
out vec3 normal;
out vec3 ps_bn;
// params that will be passed onto the fragment shader
out vec3 ps_L; // light direction
out vec3 ps_E; // eye direction (same as light direction in this simple case!)
out vec3 ps_N; // normal vector
out vec3 ps_T; // tangent vector
out vec2 ps_uv; // texture coordinate



void main()
{
	// Convert normal and position to eye coords
	 normal = normalize( normalMatrix * inNormal);
	 position = vec3(MV * vec4(inVert,1.0));


	 // transform normal into viewspace
		ps_N = (MV * vec4(inNormal, 0.0)).xyz;

		// transform normal into viewspace
		ps_T = (MV * vec4(inTangent, 0.0)).xyz;
		ps_bn = (MV * vec4(inBinormal, 0.0)).xyz;
		// compute vertex position in viewspace
		vec4 V = MV * vec4(inVert,1);

		// compute vector from light to position (done a bit wastefully here!)
		vec4 L = light.position - MV * normalize(vec4(inVert,1));

		// pass light and eye vector into fragment shader
		ps_L = L.xyz;



	 // Convert position to clip coordinates and pass along
	gl_Position = MVP*vec4(inVert,1.0);
// pass the UV values to the frag shader
vertUV=inUV.st;
}
