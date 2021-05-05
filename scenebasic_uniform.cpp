#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include "helper/texture.h"
#include "helper/particleutils.h"

#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::mat4;
using glm::vec4;
using glm::mat3;


SceneBasic_Uniform::SceneBasic_Uniform() : tPrev(0), rotSpeed(0.1f)
                                            , time(0), deltaT(0), drawBuf(1), particleLifetime(5.5f), nParticles(4000)
                                            , emitterPos(1,1,0), emitterDir(-1, 2, 0)
                                           //plane(10.0f, 10.0f, 2, 2, 1.0f, 1.0f)
{
    
    plane_1 = ObjMesh::loadWithAdjacency("media/plane.obj");
    plane_2 = ObjMesh::loadWithAdjacency("media/plane.obj");
    plane_3 = ObjMesh::loadWithAdjacency("media/plane.obj");
    plane_4 = ObjMesh::loadWithAdjacency("media/plane.obj");
    plane_5 = ObjMesh::loadWithAdjacency("media/plane.obj");
    bike = ObjMesh::loadWithAdjacency("media/model.obj");
}

void SceneBasic_Uniform::initScene()
{
    compile();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClearStencil(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    angle = 0.0f;

    //setup framebuffer object
    setupFBO();

    //setupFBO();

    shadVol.use();
    shadVol.setUniform("LightIntensity", vec3(.4f));


    //load textures
    glActiveTexture(GL_TEXTURE2);
    bikeTex = Texture::loadTexture("media/texture/bikeTex_col.png");
    bikeTexNorm = Texture::loadTexture("media/texture/download.png");

    brickTex = Texture::loadTexture("media/texture/concrete_col.jpg");
    brickTexNorm = Texture::loadTexture("media/texture/concrete_norm.jpg");


    updateLight();
    //set bike and ground shader vars
    shadVol.use();
    shadVol.setUniform("Tex", 2);
    shadVol.setUniform("TexNorm", 4);
    shadVol.setUniform("DiffSpecTex", 0);
    shadVol.setUniform("EdgeThreshold", 0.1f);


    //set smoke shader vars
    glActiveTexture(GL_TEXTURE5);
    smokeTex = Texture::loadTexture("media/smoke.png");
    glActiveTexture(GL_TEXTURE6);
    smokePart =  ParticleUtils::createRandomTex1D(nParticles * 3);

    initSmokeBuffers();

    smokeProg.use();
    smokeProg.setUniform("RandomTex", 6);
    smokeProg.setUniform("ParticleTex",5);
    smokeProg.setUniform("ParticleLifetime", particleLifetime);
    smokeProg.setUniform("Accel", vec3(0, -.5, 0));
    smokeProg.setUniform("ParticleSize", 0.05f);
    smokeProg.setUniform("Emitter", emitterPos);
    smokeProg.setUniform("EmitterBasis", ParticleUtils::makeArbitraryBasis(emitterDir));
    this->animate(true);
}

void SceneBasic_Uniform::updateLight() 
{
    lightPos = vec4(5.0f * vec3(cosf(angle) * 40.5f, 1.5f, sinf(angle) * 7.5f), 1.0f);
}


void SceneBasic_Uniform::setupFBO() 
{
    GLfloat verts[] = { -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f };

    GLfloat tc[] = {
0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };


    GLuint bufHandle;
    glGenBuffers(1, &bufHandle);
    glBindBuffer(GL_ARRAY_BUFFER, bufHandle);
    glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), verts, GL_STATIC_DRAW);

    //setup vertex array obj
    glGenVertexArrays(1, &fsQuad);
    glBindVertexArray(fsQuad);

    glBindBuffer(GL_ARRAY_BUFFER, bufHandle);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);   //vertex pos

    glBindVertexArray(0);


    //the depth buffer 
    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    //the ambient buffer
    GLuint ambBuf;
    glGenRenderbuffers(1, &ambBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, ambBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);

    //the diffuse + spec component
    glActiveTexture(GL_TEXTURE0);
    GLuint diffSpecTex;
    glGenTextures(1, &diffSpecTex);
    glBindTexture(GL_TEXTURE_2D, diffSpecTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //create and set up the FBO
    glGenFramebuffers(1, &colorDepthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, colorDepthFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ambBuf);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, diffSpecTex, 0);


    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result == GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Framebuffer is complete. \n");
    }
    else
    {
        printf("Framebuffer is not complete. \n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void  SceneBasic_Uniform::initSmokeBuffers() 
{

    glGenBuffers(2, posBuf);
    glGenBuffers(2, velBuf);
    glGenBuffers(2, age);

    // Allocate space for all buffers
    int size = nParticles * 3 * sizeof(GLfloat);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), 0, GL_DYNAMIC_COPY);

    // Fill the first age buffer
    std::vector<GLfloat> tempData(nParticles);
    float rate = particleLifetime / nParticles;

    for (int i = 0; i < nParticles; i++) {
        tempData[i] = rate * (i - nParticles);
    }

    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), tempData.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Create vertex arrays for each set of buffers
    glGenVertexArrays(2, particleArray);

    // Set up particle array
    glBindVertexArray(particleArray[0]);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);


    // Set up particle array 1
    glBindVertexArray(particleArray[1]);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);


    glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);


    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // Setup the feedback objects
    glGenTransformFeedbacks(2, feedback);
    // Transform feedback 0
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[0]);
    // Transform feedback 1
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[1]);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

}


void SceneBasic_Uniform::compile()
{
    try {
        //shader for the volumes
        volumeProg.compileShader("shader/basic_uniform.vert");
        volumeProg.compileShader("shader/basic_uniform.frag");
        volumeProg.compileShader("shader/basic_uniform.geom");
        volumeProg.link();

        shadVol.compileShader("shader/shadowVolume/shadowvolume.vert");
        shadVol.compileShader("shader/shadowVolume/shadowvolume.frag");
        shadVol.link();

        GLuint progHandle = smokeProg.getHandle();
        const char* outputNames[] = { "Position", "Velocity", "Age" };
        glTransformFeedbackVaryings(progHandle, 3, outputNames, GL_SEPARATE_ATTRIBS);

        smokeProg.compileShader("shader/smoke.vert");
        smokeProg.compileShader("shader/smoke.frag");
        smokeProg.link();
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}


float lineBrightNess;
bool valInc = true;
bool valDec = false;
float ColChangeTime = .5f;
float CurTime = 0;
void SceneBasic_Uniform::update( float t )
{
	//update your angle here
    deltaT = t - tPrev;
    
    if (tPrev == 0.0f)
    {
        deltaT = 0.0f;
    }
    
    tPrev = t;
    time = tPrev;
    if (animating())
    {
        angle += 0.2 * deltaT;

        if (angle > glm::two_pi<float>())
            angle -= glm::two_pi<float>();

        updateLight();
    }
    if (valInc)
    {
        CurTime += deltaT;
        if (CurTime >= ColChangeTime)
        {
            valDec = true;
            valInc = false;        
        }
    }

    if (valDec)
    {
        CurTime -= deltaT;
        if (CurTime <= 0)
        {
            valInc = true;
            valDec = false;
        }
    }
    
}



void SceneBasic_Uniform::render()
{
    renderSmoke();
    glFlush();
    pass1();
    glFlush();
    pass2();
    glFlush();
    pass3();
    glFlush();

}
void SceneBasic_Uniform::renderSmoke() 
{
    smokeProg.use();
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, smokeTex);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_1D, smokePart);

    smokeProg.setUniform("Time", time);
    smokeProg.setUniform("DeltaT", deltaT);

    // Update pass
    smokeProg.setUniform("pass", 1);

    glEnable(GL_RASTERIZER_DISCARD);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[drawBuf]);
    glBeginTransformFeedback(GL_POINTS);

    glBindVertexArray(particleArray[1 - drawBuf]);
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);
    glDrawArrays(GL_POINTS, 0, nParticles);
    glBindVertexArray(0);

    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);
    // Render pass
    smokeProg.setUniform("pass", 2);
    view = glm::lookAt(vec3(7.0f * cos(angle), 2.0f, 7.0f * sin(angle)), vec3(0, 2, 0), vec3(0, 1, 0));
    setMatrices(smokeProg);

    glDepthMask(GL_FALSE);
    glBindVertexArray(particleArray[drawBuf]);
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);

    drawBuf = 1 - drawBuf;

}

void SceneBasic_Uniform::pass1() 
{
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    projection = glm::infinitePerspective(glm::radians(30.0f), (float)width / height, 0.5f);
    //view = glm::lookAt(vec3(5.0f, 5.0f, 5.0f), vec3(0, 2, 0), vec3(0, 1, 0));
    view = glm::lookAt(vec3(7.0f * cos(angle), 2.0f, 7.0f * sin(angle)), vec3(0, 2, 0), vec3(0, 1, 0));

    shadVol.use();
    shadVol.setUniform("Pass", 1);
    shadVol.setUniform("LightPosition", view * lightPos);
    //renderProg.use();
    //renderProg.setUniform("LightPosition", view * lightPos);
    glBindFramebuffer(GL_FRAMEBUFFER, colorDepthFBO);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


    drawScene(shadVol, false);
    //drawScene(renderProg, false);
}

void SceneBasic_Uniform::pass2()
{

    volumeProg.use();
    volumeProg.setUniform("LightPosition", view * lightPos);

    //copy depth and color from fbo into the default fbo
    //color buffer should contain ambient component
    glBindFramebuffer(GL_READ_FRAMEBUFFER, colorDepthFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width - 1, height - 1, 0, 0, width - 1, height - 1,
        GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT, GL_NEAREST);

    //disable writing color buffer and depth buffer
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);

    //rebind to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //setup stencil test so that it always succeeds
    //increases for front faces and decreases for back faces
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xffff);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);

    //draw only shadow casters
    drawScene(volumeProg, true);

    //enable writing to color buffer

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
}

void SceneBasic_Uniform::pass3() 
{
    //dont need depth test
    glDisable(GL_DEPTH_TEST);

    //just want sum of ambient and diffuse + spec
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);


    //only render stancil pixels
    glStencilFunc(GL_EQUAL, 0, 0xffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);


    shadVol.use();
    shadVol.setUniform("Pass", 2);
    shadVol.setUniform("LineColor", vec4(1.0* (CurTime / 0.5f), .622* (CurTime / 0.5f), 0, 0));

    //compProg.use();
    //compProg.setUniform("isItGround", 1);

    model = mat4(1.0f);
    projection = model;
    view = model;
    setMatrices(shadVol);
    //setMatrices(compProg);
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

}


void SceneBasic_Uniform::drawScene(GLSLProgram& prog, bool onlyShadowCasters) 
{
    vec3 color;

    if (!onlyShadowCasters)
    {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, bikeTex);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, bikeTexNorm);

        color = vec3(1.0f);
        prog.setUniform("Ka", color * 0.1f);
        prog.setUniform("Kd", color);
        prog.setUniform("Ks", vec3(0.9f));
        prog.setUniform("Shininess", 150.0f);

        prog.setUniform("isItGround", 1);

    }

    model = mat4(1.0f);
    //model = glm::translate(model, vec3(-2.3f, 1.0f, 0.2f));
    model = glm::rotate(model, glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, vec3(.10f));
    setMatrices(prog);
    bike->render();


    if (!onlyShadowCasters)
    {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brickTex);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, brickTexNorm);
        
        color = vec3(0.5f);
        prog.setUniform("Kd", color);
        prog.setUniform("Ks", vec3(0.0f));
        prog.setUniform("Ka", vec3(0.1f));
        prog.setUniform("Shininess", 1.0f);

        //prog.setUniform("isItGround", 0);
        model = mat4(1.0f);
        model = glm::scale(model, vec3(10));
        setMatrices(prog);
        plane_1->render();


        model = mat4(1.0f);
        model = glm::translate(model, vec3(-10, 10, 0));
        model = glm::rotate(model, glm::radians(90.0f), vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(-90.0f), vec3(0, 0, 1));
        model = glm::scale(model, vec3(10));
        setMatrices(prog);
        plane_2->render();
        
        
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0, 10, -10));
        model = glm::rotate(model, glm::radians(90.0f), vec3(1, 0, 0));
        model = glm::scale(model, vec3(10));
        setMatrices(prog);
        plane_3->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(0, 0, 10));
        model = glm::rotate(model, glm::radians(-90.0f), vec3(1, 0, 0));
        model = glm::scale(model, vec3(10));
        setMatrices(prog);
        plane_4->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(10, 0, 0));
        model = glm::rotate(model, glm::radians(90.0f), vec3(0, 0, 1));
        model = glm::scale(model, vec3(10));
        setMatrices(prog);
        plane_5->render();

        model = mat4(1.0f);
    }
}



void SceneBasic_Uniform::setMatrices(GLSLProgram& prog)
{
    mat4 mv = view * model;
    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("ProjMatrix", projection);

    prog.setUniform("NormalMatrix", 
        glm::mat3( vec3(mv[0]), vec3(mv[1]), vec3(mv[2]) ));
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}
