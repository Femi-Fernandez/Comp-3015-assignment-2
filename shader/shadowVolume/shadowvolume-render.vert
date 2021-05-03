#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec3 VPosition;
out vec3 VNormal;
out vec2 VTexCoord;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjMatrix;


void main()
{
    VTexCoord = VertexTexCoord;
    VNormal = normalize( NormalMatrix * VertexNormal);
    VPosition = vec3( ModelViewMatrix * vec4(VertexPosition, 1.0) );

    gl_Position = ProjMatrix * ModelViewMatrix * vec4(VertexPosition, 1.0);
}
