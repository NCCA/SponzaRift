#version 400 core
/// @brief our output fragment colour
layout (location =0) out vec4 fragColour;
// this is a pointer to the current 2D texture object
uniform sampler2D diffuseMap;
uniform sampler2D ambientMap;
uniform sampler2D normalMap;
//uniform sampler2D shadowMap;

// varying params passed through from vertex shader
in vec3 ps_L; // light direction
in vec3 ps_N; // normal vector
in vec3 ps_T; // tangent vector
in vec2 ps_uv; // the UV texture coordinate
in vec3 ps_bn;

// the vertex UV
in vec2 vertUV;
uniform vec3 ka;
uniform vec3 kd;
in vec3 position;
in vec3 normal;
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
uniform float transp;
in  vec4  ShadowCoord;




void main()
{
 vec4 ambient = texture(ambientMap,vertUV);
 vec4 diffuse = texture(diffuseMap,vertUV);
 vec4 normalmap = texture(normalMap,vertUV);
 if (diffuse.a == 0)
   discard;


 vec3 L = normalize(ps_L);

 vec3 N = normalize(ps_N);
   vec3 T = normalize(ps_T);
   // compute bi-normal
   //vec3 B = cross(N, T);
 vec3 B = (ps_bn);
   vec3 C = normalmap.xyz;
  // grab the actual normal from the texture (which will be in texture space)
   //vec3 AN = normalize(2.0 * C - vec3(1.0, 1.0, 1.0));
  vec3 AN=C;//-normalize( texture(normalMap, vertUV.st).xyz * 2.0 - 1.0);


//	float sum;
//	sum  = textureProjOffset(shadowMap, ShadowCoord, ivec2(-1, -1)).x;
//	sum += textureProjOffset(shadowMap, ShadowCoord, ivec2( 1, -1)).x;
//	sum += textureProjOffset(shadowMap, ShadowCoord, ivec2(-1,  1)).x;
//	sum += textureProjOffset(shadowMap, ShadowCoord, ivec2( 1,  1)).x;

//	sum = sum * 1.02;



//   // rotate normal into viewspace
//   AN = T * AN.x + B * AN.y + N * AN.z;

//   // compute reflected vector
//   vec3 R = -reflect(E, AN);

   // compute N.L
   float N_dot_L = max(dot(AN, L), 0.0);
   // compute ambient lighting term
   vec3 d = light.Ld * diffuse.rgb * N_dot_L;

   fragColour.rgb = ka*ambient.rgb*light.La + kd*d;
   fragColour.a=transp;

 /*
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
 fragColour.a=transp;*/

}
