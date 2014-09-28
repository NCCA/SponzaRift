#version 400 core
/// @brief our output fragment colour
layout (location =0) out vec4 fragColour;
// this is a pointer to the current 2D texture object
uniform sampler2D ambientMap;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

// the vertex UV
in vec2 vertUV;
uniform vec3 ka;
uniform vec3 kd;
in vec3 position;
in vec3 normal;
struct LightInfo
{
	// Light position in eye coords.
	vec3 position;
	// Ambient light intensity
	vec3 La;
	// Diffuse light intensity
	vec3 Ld;
	// Specular light intensity
	vec3 Ls;
};
uniform LightInfo light;
uniform float transp;




void main()
{
 vec4 ambient = texture(ambientMap,vertUV);
 vec4 diffuse = texture(diffuseMap,vertUV);
 vec4 normalmap = texture(normalMap,vertUV);
 if (diffuse.a == 0)
   discard;

 //vec3 n = normalize(normal);
 vec3 n=normalize(normalmap.rgb * 2.0 - 1.0);
 vec3 s = normalize(light.position - position);
 vec3 v = normalize(-position);
 vec3 h = normalize( v + s );
 vec3 a = light.La * ambient.rgb;
 float sDotN = max( dot(s,normal), 0.0 );
 vec3 d = light.Ld * diffuse.rgb * sDotN;



 // set the fragment colour to the current texture
 fragColour.rgb = ka*d + kd*d;
 fragColour.a=transp;

}
