#version 460

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform vec4 LightPosition;
uniform vec3 LightIntensity;

uniform sampler2D Tex;
uniform sampler2D TexNorm;

uniform vec3 Kd;
uniform vec3 Ka;
uniform vec3 Ks;
uniform float Shininess;

layout( location = 0 ) out vec4 Ambient;
layout( location = 1 ) out vec4 DiffSpec;


void shade(vec3 n)
{
	vec3 s = normalize( vec3(LightPosition) - Position);
	vec3 v = normalize( vec3(-Position) );
	vec3 r = reflect( -s, Normal );
	vec4 texColor = texture(Tex, TexCoord);

	Ambient = vec4( texColor.rgb * LightIntensity * Ka, 1.0);
	DiffSpec = vec4( texColor.rgb * LightIntensity *
				   ( Kd * max(dot(s, n), 0.0) + 
				     Ks * pow( max( dot(r, v), 0.0), Shininess ) ),
					 1.0);
}


void main()
{	
	vec3 norm = texture(TexNorm, TexCoord).xyz;
    norm.xy = 1.0 * norm.xy - 1.0;

	shade(norm);
}