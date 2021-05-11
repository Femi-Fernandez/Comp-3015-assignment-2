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

//partical variables
uniform float Time;
uniform float DeltaT;
uniform vec3 Accel;
uniform float ParticleLifetime;
uniform vec3 Emitter;
uniform mat3 EmitterBasis;
//uniform float ParticleSize;

uniform float maxPartSize = 0.5;
uniform float minPartSize = 0.05;

//MVP 
uniform mat4 ModelViewMatrix;
uniform mat4 ProjMatrix;

uniform sampler1D RandomTex;

const float  pi = 3.14159f;
//offsets the cam coordinates for each vertex of the partical
const vec3 offsets[] = vec3[]( vec3(-.5, -.5, 0), vec3(.5, -.5, 0), vec3(.5, .5, 0),
                               vec3(-.5,-.5,0), vec3(.5, .5, 0), vec3(-.5, .5, 0));

//texture coords for each partical quad
const vec2 texCoords[] = vec2[]( vec2(0,0), vec2(1,0), vec2(1,1), vec2(0,0), vec2(1,1), vec2 (0,1) );



vec3 randomInitialVel() 
{
    float velocity = mix(0.1f, 0.5f, texelFetch(RandomTex, 2 * gl_VertexID, 0).r);
    return EmitterBasis * vec3(0, -velocity,0);
}

void update()
{ 
        Age = VertexAge + DeltaT;
    //if particle is dead or nonexistant yet, reuse/generate it
    if ((VertexAge < 0) || (VertexAge > ParticleLifetime))
    {
    //set particle position to emitter location
    //set velocity to start velocity
        Position = Emitter;
        Velocity = randomInitialVel();

        //if particle was alive but outlived age, reset its age
        if(VertexAge > ParticleLifetime)
        {
            Age = (VertexAge - ParticleLifetime) + DeltaT;
        } 
        else
        {
            //update particles position using the Euler method
            //approximates its position and speed based on predetermined values
            Position = VertexPosition + VertexVelocity *DeltaT;
            Velocity = VertexVelocity + Accel*DeltaT;
        }
    }
    else 
    {
    //update particles position using the Euler method
    //approximates its position and speed based on predetermined values
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
    //change partical size based on lifetime
        float agePerc = VertexAge/ParticleLifetime;
        posCam = (ModelViewMatrix * vec4(VertexPosition, 1)).xyz + offsets[gl_VertexID] * mix(minPartSize, maxPartSize, agePerc);
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
