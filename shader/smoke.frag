#version 460

in float Transp;
in vec2 TexCoord;

uniform sampler2D ParticleTex;
layout (location = 0) out vec4 FragColor;

void main()
{
//sets the texture and transparency
	FragColor = texture(ParticleTex, TexCoord);
	FragColor.a *= Transp;
}