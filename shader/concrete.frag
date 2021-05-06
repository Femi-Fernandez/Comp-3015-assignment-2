#version 460
in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform vec4 LightPosition;
uniform vec3 LightIntensity;

uniform vec3 Kd;
uniform vec3 Ka;
uniform vec3 Ks;
uniform float Shininess;

uniform int Pass;

uniform sampler2D Tex;
uniform sampler2D TexNorm;
uniform sampler2D DiffSpecTex;


layout (location = 0) out vec4 FragColor;


vec4 shade(vec3 n)
{
    vec3 s = normalize( vec3(LightPosition) - Position);
	vec3 v = normalize( vec3(-Position) );
	vec3 r = reflect( -s, Normal );
	vec4 texColor = texture(Tex, TexCoord);

	vec4 Ambient = vec4( texColor.rgb * LightIntensity * Ka, 1.0);
	vec4 DiffSpec = vec4( texColor.rgb * LightIntensity *
				   ( Kd * max(dot(s, n), 0.0) + 
				     Ks * pow( max( dot(r, v), 0.0), Shininess ) ),
					 1.0);
	return Ambient + DiffSpec;
}


void main() {
	//if(Pass == 1 ){
		vec3 norm = texture(TexNorm, TexCoord).xyz;
		norm.xy = 1.0 * norm.xy - 1.0;
		//shade(norm);	
	//}		

	//if(Pass == 2)
	//{
		vec4 diffSpec = texelFetch(DiffSpecTex, ivec2(gl_FragCoord), 0);
		FragColor = vec4(diffSpec.xyz, 1) + shade(norm);
	//}

}
