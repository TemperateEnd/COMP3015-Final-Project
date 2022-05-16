#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"

#include "helper/objmesh.h"
#include "helper/plane.h"

#include "helper/texture.h"
#include "helper/frustum.h"

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog, solidProg;
    GLuint shadowFBO, pass1Index, pass2Index;

    int shadowMapWidth, shadowMapHeight;
    glm::mat4 lightPV, shadowBias;
    float angle;
    float time;

    Frustum lightFrustum;

    //objects/models
    Plane plane;
    std::unique_ptr<ObjMesh> objMeshInstance;

    //textures
    GLuint swordTex = Texture::loadTexture("media/texture/swordTexture.png");
    GLuint planeTex = Texture::loadTexture("media/texture/waterTexture.png");

    void compile();
    void setMatrices();

    void setupFBO();
    void drawScene();

public:
    SceneBasic_Uniform();

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
