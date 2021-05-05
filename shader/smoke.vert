#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexVelocity;
layout (location = 2) in float VertexAge;

uniform int pass;

layout ( xfb_buffer = 0, xfb_offset = 0) out vec3 Position;
layout ( xfb_buffer = 1, xfb_offset = 0) out vec3 Velocity;
layout ( xfb_buffer = 2, xfb_offset = 0) out float Age;


out float Transp;
out vec2 TexCoord;

uniform float Time;
uniform float DeltaT;
uniform vec3 Accel;
uniform float ParticleLifetime;
uniform vec3 Emitter = vec3(0);
uniform mat3 EmitterBasis;
uniform float ParticleSize;

uniform mat4 ModelViewMatrix;
uniform mat4 ProjMatrix;

uniform sampler1D RandomTex;

const float  pi = 3.14159f;
const vec3 offsets[] = vec3[]( vec3(-.5, -.5, 0), vec3(.5, -.5, 0), vec3(.5, .5, 0),
                               vec3(-.5,-.5,0), vec3(.5, .5, 0), vec3(-.5, .5, 0));

const vec2 texCoords[] = vec2[]( vec2(0,0), vec2(1,0), vec2(1,1), vec2(0,0), vec2(1,1), vec2 (0,1) );



vec3 randomInitialVel() 
{
    //float theta = mix(0.0, pi / 8.0, texelFetch(RandomTex, 3*gl_VertexID, 0).r);
    //float phi = mix(0.0, 2.0 * pi,  texelFetch(RandomTex, 3*gl_VertexID + 1, 0).r);
    //float velocity = mix(1.25, 1.5,  texelFetch(RandomTex, 3*gl_VertexID + 2, 0).r);
    //vec3 v = vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
    //return normalize(EmitterBasis * v) * velocity;]
    float velocity = mix(0.1f, 0.5f, texelFetch(RandomTex, 2 * gl_VertexID, 0).r);
    return EmitterBasis * vec3(0, velocity, 0);
}

vec3 randomInitialPosition()
{
    float offset = mix(-2.0f, 2.0f, texelFetch(RandomTex, 2* gl_VertexID + 1, 0).r);
    return Emitter + vec3(offset, 0,0);
}

void update()
{ 
    Age = VertexAge + DeltaT;
    //if particle is dead, reuse it
    if ((VertexAge < 0) || (VertexAge > ParticleLifetime))
    {
        Position = randomInitialPosition();
        Velocity = randomInitialVel();

        if(VertexAge > ParticleLifetime)
        {
            Age = (VertexAge - ParticleLifetime) + DeltaT;
        } 
        else
        {
            Position = VertexPosition + VertexVelocity *DeltaT;
            Velocity = VertexVelocity + Accel*DeltaT;
        }
    }
    else 
    {
        Position = VertexPosition + VertexVelocity * DeltaT;
        Velocity = VertexVelocity + Accel *DeltaT;
        Age = VertexAge + DeltaT;
    }
}

void render()
{
    Transp = 0.0;
    vec3 posCam = vec3(0);
    if(VertexAge >= 0)
    {
        posCam = (ModelViewMatrix * vec4(VertexPosition, 1)).xyz + offsets[gl_VertexID] * ParticleSize;
        Transp = clamp(1.0 - VertexAge / ParticleLifetime, 0, 1);
    }
    TexCoord = texCoords[gl_VertexID];

    gl_Position = ProjMatrix * vec4(posCam, 1);
}


void main()
{
    if( pass == 1)
        update();
    if(pass == 2)
        render();
}
