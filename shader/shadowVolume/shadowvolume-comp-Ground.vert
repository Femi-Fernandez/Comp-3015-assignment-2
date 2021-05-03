#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 Position;
out vec3 Normal;
flat out int isGround;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjMatrix;
uniform int isItGround;


void main()
{
    Normal = normalize( NormalMatrix * VertexNormal);
    Position = vec3( ModelViewMatrix * vec4(VertexPosition, 1.0) );
    isGround =  isItGround;
    gl_Position = ProjMatrix * ModelViewMatrix * vec4(VertexPosition, 1.0);;
}
