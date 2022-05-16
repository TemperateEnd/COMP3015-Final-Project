#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
#include <sstream>
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include <glm/gtc/matrix_transform.hpp>

using glm::mat3;
using glm::mat4;
using glm::vec3;
using glm::vec4;

//constructor for plane and object mesh instance
SceneBasic_Uniform::SceneBasic_Uniform() :  shadowMapWidth(512), shadowMapHeight(512),
                                            time(0), plane(13.0f, 10.0f, 200, 2){
    objMeshInstance = ObjMesh::load("media/ironSword.obj", true);
}

void SceneBasic_Uniform::initScene()
{
    compile();

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    //angle = glm::quarter_pi<float>();

    setupFBO();

    GLuint programHandle = prog.getHandle();
    pass1Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "recordDepth");
    pass2Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "shadeWithShadow");

    shadowBias = mat4(vec4(0.5f, 0.0f, 0.0f, 0.0f),
        vec4(0.0f, 0.5f, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 0.5f, 0.0f),
        vec4(0.5f, 0.5f, 0.5f, 1.0f)
    );

    //setting up textures by loading file from local media subdirectory
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, swordTex);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, planeTex);

    float c = 1.65f;
    vec3 lightPos = vec3(0.0f, c * 5.25f, c * 7.5f);
    lightFrustum.orient(lightPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
    lightFrustum.setPerspective(50.0f, 1.0f, 1.0f, 25.0f);
    lightPV = shadowBias * lightFrustum.getProjectionMatrix() * lightFrustum.getViewMatrix();

    prog.setUniform("Light.Intensity", vec3(0.85f));

    prog.setUniform("ShadowMap", 0);
    angle = glm::half_pi<float>();
}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();

        solidProg.compileShader("shader/solid.vert", GLSLShader::VERTEX);
        solidProg.compileShader("shader/solid.frag", GLSLShader::FRAGMENT);
        solidProg.link();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

//updates time value for surface animation
void SceneBasic_Uniform::update( float t )
{
    time = t;
}

/**Sets Material uniforms for models before rendering them.Also sets position
and direction uniforms for light**/
void SceneBasic_Uniform::render()
{
    prog.use();

    //pass1: shadow map generation
    view = lightFrustum.getViewMatrix();
    projection = lightFrustum.getProjectionMatrix();
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass1Index);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.5f, 10.0f);
    drawScene();
    glCullFace(GL_BACK);
    glFlush();

    //pass 2: Rendering
    float c = 2.0f;
    vec3 camPos(c * 11.5f * cos(angle), c * 7.0f, c * 11.5f * sin(angle));
    view = glm::lookAt(camPos, vec3(0.0f), vec3(0.0, 1.0f, 0.0f));
    prog.setUniform("Light.Position", view * vec4(lightFrustum.getOrigin(), 1.0f));
    projection = glm::perspective(glm::radians(50.0f), (float)width / height, 0.1f, 100.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass2Index);
    drawScene();

    solidProg.use();
    solidProg.setUniform("Color", vec4(1.0f, 0.0f, 0.0f, 1.0f));
    mat4 mv = view * lightFrustum.getInverseViewMatrix();
    solidProg.setUniform("MVP", projection * mv);
}

void SceneBasic_Uniform::drawScene()
{
    prog.setUniform("Time", time);
    prog.setUniform("isAnimated", false); //as model won't be animated, this should be false
    prog.setUniform("Material.Kd", 0.2f, 0.5f, 0.9f);
    prog.setUniform("Material.Ka", 0.2f, 0.5f, 0.9f);
    prog.setUniform("Material.Shininess", 250.0f);
    prog.setUniform("Tex1", 0);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(180.0f), vec3(1.0f, 1.0f, 1.0f));
    model = glm::translate(model, vec3(9.0f, 5.0f, 0.0f));
    model = glm::scale(model, vec3(3.0f, 3.0f, 3.0f));
    setMatrices();
    objMeshInstance->render(); //renders sword
    
    prog.setUniform("isAnimated", true); //surface of this model will be animated, so true
    prog.setUniform("Material.Kd", 0.2f, 0.5f, 0.9f);
    prog.setUniform("Material.Ka", 0.2f, 0.5f, 0.9f);
    prog.setUniform("Material.Shininess", 100.0f);
    prog.setUniform("Tex1", 1);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 0.0f, 7.0f));
    setMatrices();
    plane.render();

    prog.setUniform("isAnimated", false); //surface of this model will be animated, so true
    prog.setUniform("Material.Kd", 0.2f, 0.5f, 0.9f);
    prog.setUniform("Material.Ka", 0.2f, 0.5f, 0.9f);
    prog.setUniform("Material.Shininess", 1.0f);
    prog.setUniform("Tex1", 1);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    model = glm::translate(model, vec3(0.0f, 5.0f, -5.0f));
    setMatrices();
    plane.render();
    model = mat4(1.0f);
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

/** Takes all matrices needed in shaders **/
void SceneBasic_Uniform::setMatrices()
{
    mat4 mv = view * model;
    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix", 
        glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    prog.setUniform("MVP", projection * mv);
    prog.setUniform("ShadowMatrix", lightPV * model);
}

void SceneBasic_Uniform::setupFBO()
{
    GLfloat border[] = { 1.0f, 0.0f, 0.0f, 0.0f };

    GLuint depthTex;

    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, shadowMapWidth, shadowMapHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTex);

    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, depthTex, 0);

    GLenum drawBuffers[] = { GL_NONE };
    glDrawBuffers(1, drawBuffers);

    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result == GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer complete\n");
    }
    else {
        printf("Framebuffer incomplete\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
