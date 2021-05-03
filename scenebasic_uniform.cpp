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


#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::mat4;
using glm::vec4;
using glm::mat3;


SceneBasic_Uniform::SceneBasic_Uniform() : tPrev(0), rotSpeed(0.1f)
                                           //plane(10.0f, 10.0f, 2, 2, 1.0f, 1.0f)
{
    
    plane_1 = ObjMesh::loadWithAdjacency("media/plane.obj");
    plane_2 = ObjMesh::loadWithAdjacency("media/plane.obj");
    plane_3 = ObjMesh::loadWithAdjacency("media/plane.obj");
    spot = ObjMesh::loadWithAdjacency("media/model.obj");
}

void SceneBasic_Uniform::initScene()
{
    compile();

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClearStencil(0);

    glEnable(GL_DEPTH_TEST);

    angle = 0.0f;

    //setup framebuffer object
    setupFBO();

    shadVol.use();
    shadVol.setUniform("LightIntensity", vec3(1.0f));
    //renderProg.use();
    //renderProg.setUniform("LightIntensity", vec3(1.0f));

    //setup VAO for fullscreen quad
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



    //load textures
    glActiveTexture(GL_TEXTURE2);
    bikeTex = Texture::loadTexture("media/texture/bikeTex_col.png");
    bikeTexNorm = Texture::loadTexture("media/texture/download.png");

    brickTex = Texture::loadTexture("media/texture/concrete_col.jpg");
    brickTexNorm = Texture::loadTexture("media/texture/concrete_norm.jpg");
    //brickTex = Texture::loadTexture("media/spot/spot_texture.png");

    updateLight();

    shadVol.use();
    shadVol.setUniform("Tex", 2);
    shadVol.setUniform("TexNorm", 4);
    //renderProg.use();
    //renderProg.setUniform("Tex", 2);
    //renderProg.setUniform("TexNorm", 4);
    //renderProg.setUniform("EdgeWidth", .01f);
    //renderProg.setUniform("PctExtend", .05f);
    //renderProg.setUniform("LineColor", vec4(1.0, .622, 0, 1.0));
    //renderProg.setUniform("EdgeThreshold", 0.005f);


    shadVol.use();
    shadVol.setUniform("DiffSpecTex", 0);
    shadVol.setUniform("EdgeThreshold", 0.05f);
    //compProg.use();
    //compProg.setUniform("DiffSpecTex", 0);
    //compProg.setUniform("EdgeThreshold", 0.05f);

    this->animate(true);
}

void SceneBasic_Uniform::updateLight() 
{
    lightPos = vec4(5.0f * vec3(cosf(angle) * 7.5f, 1.5f, sinf(angle) * 7.5f), 1.0f);
}


void SceneBasic_Uniform::setupFBO() 
{
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

void SceneBasic_Uniform::compile()
{
    try {
        //shader for the volumes
        volumeProg.compileShader("shader/basic_uniform.vert");
        volumeProg.compileShader("shader/basic_uniform.frag");
        volumeProg.compileShader("shader/basic_uniform.geom");
        volumeProg.link();
        //prog.use();

        //shader for rendering and compositing
        renderProg.compileShader("shader/shadowVolume/shadowvolume-render.vert");
        //renderProg.compileShader("shader/shadowVolume/shadowvolume-render.geom");
        renderProg.compileShader("shader/shadowVolume/shadowvolume-render.frag");
        renderProg.link();
        
        compProg.compileShader("shader/shadowVolume/shadowvolume-comp.vert");
        compProg.compileShader("shader/shadowVolume/shadowvolume-comp.frag");
        compProg.link();

        shadVol.compileShader("shader/shadowVolume/shadowvolume.vert");
        shadVol.compileShader("shader/shadowVolume/shadowvolume.frag");
        shadVol.link();
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}


void SceneBasic_Uniform::update( float t )
{
	//update your angle here
    float deltaT = t - tPrev;
    if (tPrev == 0.0f)
    {
        deltaT = 0.0f;
    }
    
    tPrev = t;
    
    if (animating())
    {
        angle += 0.2 * deltaT;

        if (angle > glm::two_pi<float>())
            angle -= glm::two_pi<float>();

        updateLight();
    }
   
}



void SceneBasic_Uniform::render()
{
    pass1();
    glFlush();
    pass2();
    glFlush();
    pass3();


}

void SceneBasic_Uniform::pass1() 
{
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    projection = glm::infinitePerspective(glm::radians(30.0f), (float)width / height, 0.5f);
    view = glm::lookAt(vec3(5.0f, 5.0f, 5.0f), vec3(0, 2, 0), vec3(0, 1, 0));

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
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glStencilFunc(GL_EQUAL, 0, 0xffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);


    shadVol.use();
    shadVol.setUniform("pass", 2);
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


    }

    model = mat4(1.0f);
    //model = glm::translate(model, vec3(-2.3f, 1.0f, 0.2f));
    model = glm::rotate(model, glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, vec3(1.0f));
    setMatrices(prog);
    spot->render();


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

        model = mat4(1.0f);
        model = glm::scale(model, vec3(6));
        setMatrices(prog);
        plane_1->render();


        model = mat4(1.0f);
        model = glm::translate(model, vec3(-5.0f, 5.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, vec3(6));
        setMatrices(prog);
        plane_2->render();


        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, 5.0f, -5.0f));
        model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(6));
        setMatrices(prog);
        plane_3->render();

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
