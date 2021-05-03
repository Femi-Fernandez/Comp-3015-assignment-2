#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>

#include "helper/glslprogram.h"
#include "helper/Objmesh.h"
#include "helper/torus.h"
#include "helper/plane.h"
#include "helper/frustum.h"
class SceneBasic_Uniform : public Scene
{
private:
    
    GLSLProgram volumeProg, renderProg, compProg, shadVol;
    GLuint colorDepthFBO, fsQuad;
    GLuint bikeTex, bikeTexNorm, brickTex, brickTexNorm;

   // Plane plane;
    std::unique_ptr<ObjMesh> spot;
    std::unique_ptr<ObjMesh> plane_1, plane_2, plane_3;


    glm::vec4 lightPos;

    float angle, tPrev, rotSpeed;

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
