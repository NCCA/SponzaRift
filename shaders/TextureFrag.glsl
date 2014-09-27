#version 400 core
/// @brief our output fragment colour
layout (location =0) out vec4 fragColour;
// this is a pointer to the current 2D texture object
uniform sampler2D tex;
uniform sampler2D mask;

// the vertex UV
in vec2 vertUV;
uniform vec3 ka;
uniform float transp;
void main ()
{
 vec4 maskValue=texture(mask,vertUV);
 if (maskValue.r ==0.0)
   discard;
 // set the fragment colour to the current texture
 fragColour = vec4(ka,transp)*texture(tex,vertUV);

}
