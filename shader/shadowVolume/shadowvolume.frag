#version 460

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

//light variables
uniform vec4 LightPosition;
uniform vec3 LightIntensity;

//input for texture, textur normal and shading of the models
uniform sampler2D Tex;
uniform sampler2D TexNorm;
uniform sampler2D DiffSpecTex;

//material variables
uniform vec3 Kd;
uniform vec3 Ka;
uniform vec3 Ks;
uniform float Shininess;

//edge Detection variables
uniform float EdgeThreshold;
const vec3 lum = vec3(0.2126, 0.7152, 0.0722);
uniform vec4 LineColor;

//pass to control what is rendered when
uniform int Pass;

//output color
layout( location = 0 ) out vec4 FragColor;

//blinn-phong shading
void shade(vec3 n)
{
	vec3 s = normalize( vec3(LightPosition) - Position);
	vec3 v = normalize( vec3(-Position) );
	vec3 r = reflect( -s, Normal );
	vec4 texColor = texture(Tex, TexCoord);
	//calculate ambient
	vec4 Ambient = vec4( texColor.rgb * LightIntensity * Ka, 1.0);

	//calculate the diffuse and specular
	vec4 DiffSpec = vec4( texColor.rgb * LightIntensity *
				   ( Kd * max(dot(s, n), 0.0) + 
				     Ks * pow( max( dot(r, v), 0.0), Shininess ) ),
					 1.0);
	FragColor = Ambient + DiffSpec;
}

//edge detection
float luminance( vec3 color )
{
    return dot(lum,color);
}

vec4 outlineCalc()
{
	ivec2 pix = ivec2(gl_FragCoord.xy); //we grab a pixel to check if edge
	//pick neighboutring pixels for convolution kernel
	float s00 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(-1,1)).rgb);
	float s10 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(-1,0)).rgb);
	float s20 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(-1,-1)).rgb);
	float s01 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(0,1)).rgb);
	float s21 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(0,-1)).rgb);
	float s02 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(1,1)).rgb);
	float s12 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(1,0)).rgb);
	float s22 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(1,-1)).rgb);
	float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
	float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);
	float g = sx * sx + sy * sy;
	if( g > EdgeThreshold )
	return LineColor; //edge
	else
	return vec4(0.0,0.0,0.0,1.0); //no edge
}

	


void main()
{	
	if(Pass == 1 ){
	//just render the shading
		vec3 norm = texture(TexNorm, TexCoord).xyz;
		norm.xy = 1.0 * norm.xy - 1.0;
		shade(norm);	
	}		

	if(Pass == 2)
	{
	//add the texture color and edge colors
		vec4 diffSpec = texelFetch(DiffSpecTex, ivec2(gl_FragCoord), 0);
		FragColor = vec4(diffSpec.xyz, 1) + outlineCalc();
	}
}

