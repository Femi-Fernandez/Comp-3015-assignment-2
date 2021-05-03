#version 460

in vec3 Position;
in vec3 Normal;

uniform sampler2D DiffSpecTex;

layout( location = 0 ) out vec4 FragColor;

//uniform float EdgeThreshold;
//const vec3 lum = vec3(0.2126, 0.7152, 0.0722);
//
//
//float luminance( vec3 color )
//{
//    return dot(lum,color);
//}
//
//vec4 outlineCalc()
//{
//	ivec2 pix = ivec2(gl_FragCoord.xy); //we grab a pixel to check if edge
//	//pick neighboutring pixels for convolution filter
//	//check lecture slides
//	float s00 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(-1,1)).rgb);
//	float s10 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(-1,0)).rgb);
//	float s20 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(-1,-1)).rgb);
//	float s01 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(0,1)).rgb);
//	float s21 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(0,-1)).rgb);
//	float s02 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(1,1)).rgb);
//	float s12 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(1,0)).rgb);
//	float s22 = luminance(texelFetchOffset(DiffSpecTex, pix, 0, ivec2(1,-1)).rgb);
//	float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
//	float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);
//	float g = sx * sx + sy * sy;
//	if( g > EdgeThreshold )
//	return vec4(1.0, .622, 0, 1.0); //edge
//	else
//	return vec4(0.0,0.0,0.0,1.0); //no edge
//}
//

void main()
{

	vec4 diffSpec = texelFetch(DiffSpecTex, ivec2(gl_FragCoord), 0);

	FragColor = vec4(diffSpec.xyz, 1);
	//FragColor = vec4(diffSpec.xyz, 1) + outlineCalc();
}