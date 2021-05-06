#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>

#include "helper/glslprogram.h"
#include "helper/Objmesh.h"
#include "helper/torus.h"
#include "helper/plane.h"
#include "helper/random.h"
//#include "helper/frustum.h"

class SceneBasic_Uniform : public Scene
{
private:
    
    //bike & ground
    GLSLProgram volumeProg, shadVol, concProg;
    GLuint colorDepthFBO, fsQuad;
    GLuint bikeTex, bikeTexNorm, brickTex, brickTexNorm, smokeTex, smokePart;

    std::unique_ptr<ObjMesh> bike;
    std::unique_ptr<ObjMesh> plane_1, plane_2, plane_3, plane_4, plane_5;


    glm::vec4 lightPos;

    float angle, angleCount,  tPrev, rotSpeed;
    int lightingType;

    //smoke 
    GLSLProgram smokeProg;
    Random rand;
    GLuint  posBuf[2], velBuf[2], age[2], particleArray[2], feedback[2];

    GLuint drawBuf;

    glm::vec3 emitterPos, emitterDir;
    float particleLifetime;
    int nParticles;

    float time, deltaT;
    bool concClear;
    void initSmokeBuffers();
    void renderSmoke();
    //void randFloat();
    void renderConc();


    void setMatrices(GLSLProgram &);

    void compile();

    void setupFBO();
    void drawScene(GLSLProgram &, bool);
    void pass1();
    void pass2();
    void pass3();
    void updateLight();


    //void spitOutDepthBuffer();

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
