// Local Headers

#include "glitter.hpp"
#include <shader.h>
#include <model.h>
#include <tools_base.h>
// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cube_base.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Program.hpp>
// Standard Headers
#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <stdexcept>
#include <cmath>
#include <climits>

extern float vertices[288];
void setCube(float* vertices);
unsigned int loadTexture(std::vector< std::string> faces);

unsigned int loadTexture(char const* path);
bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;
// settings
extern Camera camera;
unsigned int quadVAO = 0;
unsigned int quadVBO;



//water
const unsigned SAMPLES = 8;
const unsigned TEXTURES_AMOUNT = 13;
const unsigned TESS_LEVEL = 16; //对四边形进行更细的划分
const float DEPTH = 0.2f;
unsigned heightMap[TEXTURES_AMOUNT];
unsigned normalMap[TEXTURES_AMOUNT];
unsigned waterTex;
unsigned wavesNormalMap;
unsigned wavesHeightMap;
unsigned noise;
unsigned firstIndex = 0;
unsigned lastIndex = 1;
bool rotate = false;
unsigned VAO = 0;
float interpolateFactor = 0.0f;
Program program;
void initProgram();
void initGL();
void renderWater();
//framebuffer
unsigned int framebuffer_water, textureColorbuffer, textureDepthbuffer;
void initFramebuffer();


//cube
unsigned int cubeVBO, cubeVAO;
void initCube();
void renderCube();
void renderCube2();
Program cubeprogram;


void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


//lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
int main(int argc, char * argv[]) {
    extern float cude_index[108];
    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(mWindow, mouse_callback);
    glfwSetScrollCallback(mWindow, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
// -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);


    glGenVertexArrays(1, &VAO);
    glGenTextures(TEXTURES_AMOUNT, heightMap);
    glGenTextures(TEXTURES_AMOUNT, normalMap);
    glGenTextures(1, &waterTex);
    for (unsigned i = 0; i < TEXTURES_AMOUNT; ++i) {
        std::string num = std::to_string(i + 1);
        heightMap[i] = loadTexture(("../Glitter/objects/heights/" + num + ".png").c_str());
        normalMap[i] = loadTexture(("../Glitter/objects/normals/" + num + ".png").c_str());
    }
    waterTex = loadTexture("../Glitter/objects/water.jpg");
    wavesNormalMap = loadTexture("../Glitter/objects/wavesNormal.jpg");
    wavesHeightMap = loadTexture("../Glitter/objects/wavesHeight.jpg");
    noise = loadTexture("../Glitter/objects/noise.png");
    initCube();
    initFramebuffer();
    // render loop


    // build and compile shaders
    // -------------------------
    Shader screenShader("../Glitter/screen_shader.vs", "../Glitter/screen_shader.fs");
    Shader ourShaderSingle("../Glitter/single_color.vs", "../Glitter/single_color.fs");
    Shader ourShader("../Glitter/1.model_loading.vs", "../Glitter/1.model_loading.fs");
    Shader ourShader2("../Glitter/1.model_loading.vs", "../Glitter/model_loading.fs");
    Shader lightCubeShader("../Glitter/light_cube.vs", "../Glitter/lightt_cube.fs");
    Shader skyBoxShader("../Glitter/skybox.vs", "../Glitter/skybox.fs");
    Shader shaderBlur("../Glitter/7.blur.vs", "../Glitter/7.blur.fs");
    Shader shaderBloomFinal("../Glitter/7.bloom_final.vs", "../Glitter/7.bloom_final.fs");
    // load models
    // -----------
   /* char* rawobj = your_filereading_function("file/name.obj");

    objgl2StreamInfo strinfo = objgl2_init_bufferstream(rawobj);
    objgl2Data objd = objgl2_readobj(&strinfo);*/
    Model ourModel("../Glitter/objects/nanosuit.obj");
    unsigned int VBO, cubeVAO;
    glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3(0.0f,  0.0f, -3.0f)
    };
    
    setCube(vertices);
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
   
    unsigned int skyVAO,skyVBO;
    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);

    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cude_index), cude_index, GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);



    //skybox
    // 
    unsigned int skyboxTexture;
    std::vector< std::string> faces;
    faces.push_back("../Glitter/objects/left.jpg");
    faces.push_back("../Glitter/objects/right.jpg");

    faces.push_back("../Glitter/objects/top.jpg");
    faces.push_back("../Glitter/objects/bottom.jpg");
    faces.push_back("../Glitter/objects/front.jpg");
    faces.push_back("../Glitter/objects/back.jpg");
    skyboxTexture = loadTexture(faces);

    skyBoxShader.use();
    skyBoxShader.setInt("sky", 0);
    // Rendering Loop

   // configure (floating point) framebuffers
    // ---------------------------------------

        //帧缓冲 
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // 生成纹理
    unsigned int texColorBuffer;
    glGenTextures(1, &texColorBuffer);

    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);


    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;


    glBindFramebuffer(GL_FRAMEBUFFER, 0);





    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }


    shaderBloomFinal.use();
    shaderBloomFinal.setInt("scene", 0);
    shaderBloomFinal.setInt("bloomBlur", 1);
    shaderBlur.use();
    shaderBlur.setInt("image", 0);
    while (glfwWindowShouldClose(mWindow) == false) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(mWindow);
        // input
     //glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
     //   glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
     //   glClearColor(1.0f,1.0f, 1.0f, 1.0f);
     //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // don't forget to clear the stencil buffer!

     //   //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
     //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // don't forget to clear the stencil buffer!



     //   // -----

     //   glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
     //   glm::mat4 view = camera.GetViewMatrix();
     //   glm::mat4 model = glm::mat4(1.0f);

     //   ourShader2.use();
     //   ourShader2.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
     //   // material properties
     //   ourShader2.setFloat("material.shininess", 32.0f);
     //   // view/projection transformations

     //   ourShader2.setMat4("projection", projection);
     //   ourShader2.setMat4("view", view);
     //   ourShader2.setVec3("cameraPos", camera.Position);

     //   // render the loaded model
     //   model = glm::translate(model, glm::vec3(10.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
     //   model = glm::scale(model, glm::vec3(1.0f));	// it's a bit too big for our scene, so scale it down
     //   ourShader2.setMat4("model", model);
     //   ourModel.Draw(ourShader2);
     //   ourShaderSingle.use();
     //   model = glm::mat4(1.0f);
     //   view = camera.GetViewMatrix();
     //   projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
     //   ourShaderSingle.setMat4("view", view);
     //   ourShaderSingle.setMat4("projection", projection);

     //   // don't forget to enable shader before setting uniforms
     //   ourShader.use();

     //   // directional light
     //   ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
     //   ourShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
     //   ourShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
     //   ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
     //   // point light 1
     //   ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
     //   ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
     //   ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
     //   ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
     //   ourShader.setFloat("pointLights[0].constant", 1.0f);
     //   ourShader.setFloat("pointLights[0].linear", 0.09f);
     //   ourShader.setFloat("pointLights[0].quadratic", 0.032f);
     //   // point light 2
     //   ourShader.setVec3("pointLights[1].position", pointLightPositions[1]);
     //   ourShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
     //   ourShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
     //   ourShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
     //   ourShader.setFloat("pointLights[1].constant", 1.0f);
     //   ourShader.setFloat("pointLights[1].linear", 0.09f);
     //   ourShader.setFloat("pointLights[1].quadratic", 0.032f);
     //   // point light 3
     //   ourShader.setVec3("pointLights[2].position", pointLightPositions[2]);
     //   ourShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
     //   ourShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
     //   ourShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
     //   ourShader.setFloat("pointLights[2].constant", 1.0f);
     //   ourShader.setFloat("pointLights[2].linear", 0.09f);
     //   ourShader.setFloat("pointLights[2].quadratic", 0.032f);
     //   // point light 4
     //   ourShader.setVec3("pointLights[3].position", pointLightPositions[3]);
     //   ourShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
     //   ourShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
     //   ourShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
     //   ourShader.setFloat("pointLights[3].constant", 1.0f);
     //   ourShader.setFloat("pointLights[3].linear", 0.09f);
     //   ourShader.setFloat("pointLights[3].quadratic", 0.032f);
     //   // spotLight
     //   ourShader.setVec3("spotLight.position", camera.Position);
     //   ourShader.setVec3("spotLight.direction", camera.Front);
     //   ourShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
     //   ourShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
     //   ourShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
     //   ourShader.setFloat("spotLight.constant", 1.0f);
     //   ourShader.setFloat("spotLight.linear", 0.09f);
     //   ourShader.setFloat("spotLight.quadratic", 0.032f);
     //   ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
     //   ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));


     //   ourShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
     //   // material properties
     //   ourShader.setFloat("material.shininess", 64.0f);
     //   // view/projection transformations
     //   projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
     //   view = camera.GetViewMatrix();
     //   ourShader.setMat4("projection", projection);
     //   ourShader.setMat4("view", view);

     //   // draw floor as normal, but don't write the floor to the stencil buffer, we only care about the containers. We set its mask to 0x00 to not write to the stencil buffer.

     //   // render the loaded model
     //   model = glm::mat4(1.0f);
     //   model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
     //   model = glm::scale(model, glm::vec3(1.0f));	// it's a bit too big for our scene, so scale it down
  
     //   model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
     //   ourShader.setMat4("model", model);

     //   glStencilMask(0x00);
     //   glStencilFunc(GL_ALWAYS, 1, 0xFF);
     //   glStencilMask(0xFF);
     //   ourModel.Draw(ourShader);
     //   //泛光收尾函数
     //   glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
     //   glStencilMask(0x00);
     //   glDisable(GL_DEPTH_TEST);
     //   ourShaderSingle.use();
     //   ourShaderSingle.setMat4("model", model);

     //   
     //   // glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

     //    //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

     //  // glActiveTexture(GL_TEXTURE0);

     //   ourModel.Draw(ourShaderSingle);
     //   glBindVertexArray(0);
     //   glStencilMask(0xFF);
     //   glStencilFunc(GL_ALWAYS, 0, 0xFF);

     //   glEnable(GL_DEPTH_TEST);
     //   //天空盒后画，减少损耗
     //   glDepthFunc(GL_LEQUAL);
     //   skyBoxShader.use();
     //   // 传视图,投影矩阵
     //   view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
     //   skyBoxShader.setMat4("view", view);
     //   skyBoxShader.setMat4("projection", projection);
     //   // skybox cube
     //   glBindVertexArray(skyVAO);
     //   glActiveTexture(GL_TEXTURE0);
     //   glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
     //   glDrawArrays(GL_TRIANGLES, 0, 36);
     //   glBindVertexArray(0);
     //   glDepthFunc(GL_LESS); // set depth function back to default


     //   glBindFramebuffer(GL_FRAMEBUFFER, 0);
     //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

     //          // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
     //   //绑定回初始缓冲


     //   bool horizontal = true, first_iteration = true;
     //   unsigned int amount = 10;
     //   shaderBlur.use();
     //   for (unsigned int i = 0; i < amount; i++)
     //   {
     //       glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
     //       shaderBlur.setInt("horizontal", horizontal);
     //       glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
     //       renderQuad();
     //       horizontal = !horizontal;
     //       if (first_iteration)
     //           first_iteration = false;
     //   }

     //   glBindFramebuffer(GL_FRAMEBUFFER, 0);



     //   // clear all relevant buffers
     //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


     //   shaderBloomFinal.use();
     //   glActiveTexture(GL_TEXTURE0);
     //   glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
     //   glActiveTexture(GL_TEXTURE1);
     //   glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
     //   shaderBloomFinal.setInt("bloom", bloom);
     //   shaderBloomFinal.setFloat("exposure", exposure);
     //   renderQuad();

glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_water);
glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
renderCube2();
// render
glBindFramebuffer(GL_FRAMEBUFFER, 0);
glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// view/projection transformations
renderCube();
renderWater();

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   glfwTerminate();
    return EXIT_SUCCESS;
}



void setCube(float* vertices)
{

}




unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


//skybox
// 
// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void initProgram() {
    program.create();
    program.attachShader(createVertexShader("../Glitter/water-vert.vs"));
    program.attachShader(createTessalationControlShader("../Glitter/water-tess-control.glsl"));
    program.attachShader(createTessalationEvaluationShader("../Glitter/water-tess-eval.glsl"));
    program.attachShader(createFragmentShader("../Glitter/water-frag.fs"));
    program.link();
    program.use();
    program.setVec3("light.direction", glm::vec3(0.0, -1.0, 0.0));
    program.setVec3("light.ambient", glm::vec3(0.3, 0.3, 0.3));
    program.setVec3("light.diffuse", glm::vec3(0.75, 0.75, 0.75));
    program.setVec3("light.specular", glm::vec3(1.0, 1.0, 1.0));
    program.setFloat("interpolateFactor", interpolateFactor);
    program.setFloat("depth", DEPTH);
    program.setInt("tessLevel", TESS_LEVEL);
}
int worldTime = 12000;
void renderWater() {
    glBindVertexArray(VAO);
    program.use();
    program.setInt("heightMap1", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightMap[firstIndex]);

    program.setInt("heightMap2", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heightMap[lastIndex]);

    program.setInt("normalMap1", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalMap[firstIndex]);

    program.setInt("normalMap2", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, normalMap[lastIndex]);

    program.setInt("water", 4);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, waterTex);

    program.setInt("wavesHeightMap", 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, wavesHeightMap);

    program.setInt("wavesNormalMap", 6);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, wavesNormalMap);

    program.setInt("reflectMap", 7);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    program.setInt("depthMap", 8);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, textureDepthbuffer);
    program.setInt("noise", 9);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, noise);

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 1.0f, 3.0f));
    program.use();
    program.setMat4("projection", projection);
    program.setMat4("view", view);
    program.setMat4("model", model);
    worldTime = (worldTime + 1) % 24000;
    program.setInt("worldTime", worldTime);
    program.setVec3("viewPos", camera.Position);
    if (interpolateFactor >= 0.7)
    {
        interpolateFactor = 0.3f;
        if (lastIndex == TEXTURES_AMOUNT - 1)
        {
            firstIndex = 0;
            lastIndex = 1;
        }
        else
        {
            ++firstIndex;
            ++lastIndex;
        }
    }
    else
    {
        interpolateFactor += 0.03 * deltaTime;
        program.setFloat("interpolateFactor", interpolateFactor);
    }

    static float offset = 0.0f;
    if (offset >= INT_MAX - 2)
        offset = 0;
    offset += 0.2 * deltaTime;
    program.setFloat("wavesOffset", offset);

    glBindVertexArray(VAO);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void initCube() {
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);//绑定VAO
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);//绑定VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//传递顶点数据
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);//设定属性指针
    glEnableVertexAttribArray(0);
    cubeprogram.create();
    cubeprogram.attachShader(createVertexShader("../Glitter/cube.vs"));
    cubeprogram.attachShader(createFragmentShader("../Glitter/cube.fs"));
    cubeprogram.link();
}
void renderCube() {
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
    cubeprogram.use();
    cubeprogram.setMat4("model", model);
    cubeprogram.setMat4("view", view);
    cubeprogram.setMat4("projection", projection);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void initFramebuffer() {
    glGenFramebuffers(1, &framebuffer_water);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_water);
    // create a color attachment texture
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a depth attachment texture
    glGenTextures(1, &textureDepthbuffer);
    glBindTexture(GL_TEXTURE_2D, textureDepthbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepthbuffer, 0);
}
void renderCube2() {
    glm::vec3 rPos = glm::vec3(camera.Position.x, -camera.Position.y, camera.Position.z);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::vec3 Front = glm::vec3(camera.Front.x, -camera.Front.y, camera.Front.z);
    glm::mat4 view = glm::lookAt(camera.Position, camera.Position + glm::vec3(camera.Front.x, 0.0, camera.Front.z), camera.Up);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
    cubeprogram.use();
    cubeprogram.setMat4("model", model);
    cubeprogram.setMat4("view", view);
    cubeprogram.setMat4("projection", projection);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
