// Local Headers

#include "glitter.hpp"
#include <shader.h>

#include <model_animation.h>
#include <animator.h>
//#include <model.h>
#include <camera.h>
// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//#include <cube_base.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Standard Headers
#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <stdexcept>
#include <cmath>
#include <climits>

//waterheaders
#include<QuadScreen.hpp>
#include<FrameBuffer.hpp>
#include<DepthFramebuffer.hpp>
#include<Water.hpp>
#include<Cube.hpp>
#include<particle.hpp>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(std::vector<std::string> faces);
unsigned int loadTexture(char const* path);
void initGL();
void initWaterPart(Shader* cubeShader,Shader* waterShader,Shader* quadShader);
void renderwater(Shader* ourShader, Model* ourModel, Shader* shaderBlur);
glm::vec3 pointLightPositions[] = {
glm::vec3(0.7f,  0.2f,  2.0f),
glm::vec3(2.3f, -3.3f, -4.0f),
glm::vec3(-4.0f,  2.0f, -12.0f),
glm::vec3(0.0f,  0.0f, -3.0f)
};




//window
GLFWwindow* window;
QuadScreen quad_screen;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 2.5f, 2.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 light_pos(0.0f, 0.0f, 0.0f);
glm::vec3 camera_position;
glm::vec3 camera_direction;

GLuint light_mode_selected;
GLuint light_mode = 0;
GLuint water_effect = 0;

//  framebuffer
Framebuffer* framebuffer_refraction;
Framebuffer* framebuffer;
Framebuffer* framebuffer_reflection;
DepthFramebuffer* depth_framebuffer;

unsigned int pingpongFBO[2];
unsigned int pingpongColorbuffers[2];
unsigned int colorBuffers[2];
unsigned int hdrFBO;
unsigned int particle_buffer;
unsigned int textureColorbuffer;
//  model frame buffer

unsigned int quadVAO = 0;
unsigned int quadVBO;
bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;



void renderQuad();
//  objects
Cube cube_base[5];
Cube cube_decoration[2];
Water water;
GLuint effect_select;

std::vector<Drawable*> Drawable_list;

//  transforms
glm::mat4 cube_base_trans[5];
glm::mat4 cube_deco_trans[2];
glm::mat4 water_trans;
glm::mat4 projection = glm::perspective(3.1415f / 2.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
void initModel(Shader* ourShader);
//ParticleGenerator* Particles;
//const int MaxParticles = 100000;
Particle ParticlesContainer[MaxParticles];
int LastUsedParticle = 0;

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int FindUnusedParticle() {

    for (int i = LastUsedParticle; i < MaxParticles; i++) {
        if (ParticlesContainer[i].life < 0) {
            LastUsedParticle = i;
            return i;
        }
    }

    for (int i = 0; i < LastUsedParticle; i++) {
        if (ParticlesContainer[i].life < 0) {
            LastUsedParticle = i;
            return i;
        }
    }

    return 0; // All particles are taken, override the first one
}

void SortParticles() {
    std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}











int main()
{
    initGL();

    // build and compile shaders
    // -------------------------
    Shader screenShader("../Glitter/Shaders/screen_shader.vs", "../Glitter/Shaders/screen_shader.fs");
    Shader ourShaderSingle("../Glitter/Shaders/single_color.vs", "../Glitter/Shaders/single_color.fs");
    Shader ourShader("../Glitter/Shaders/1.model_loading.vs", "../Glitter/Shaders/1.model_loading.fs");
    Shader ourShader2("../Glitter/Shaders/1.model_loading.vs", "../Glitter/Shaders/model_loading.fs");
    Shader lightCubeShader("../Glitter/Shaders/light_cube.vs", "../Glitter/Shaders/lightt_cube.fs");
    Shader skyBoxShader("../Glitter/Shaders/skybox.vs", "../Glitter/Shaders/skybox.fs");
    Shader shaderBlur("../Glitter/Shaders/7.blur.vs", "../Glitter/Shaders/7.blur.fs");
    Shader shaderBloomFinal("../Glitter/Shaders/7.bloom_final.vs", "../Glitter/Shaders/7.bloom_final.fs");
    
    //watershader
    Shader cubeShader("../Glitter/Shaders/cube.vs", "../Glitter/Shaders/cube.fs");
    Shader waterShader("../Glitter/Shaders/water_vshader.glsl", "../Glitter/Shaders/water_fshader.glsl");
    Shader quadShader("../Glitter/Shaders/quad_screen_vshader.glsl", "../Glitter/Shaders/quad_screen_fshader.glsl");
    // load models
    // -----------
    //Model ourModel("../Glitter/objects/nanosuit.obj");
    Model ourModel("../Glitter/objects/final/Guppy.fbx");
    Animation Animation("../Glitter/objects/final/Guppy.fbx", &ourModel);
    Animator animator(&Animation);




    unsigned int VBO, cubeVAO;

    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)

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


    //// ping-pong-framebuffer for blurring

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


    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &particle_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, particle_buffer);

    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);


    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("../Glitter/Shaders/Particle.vertexshader", "../Glitter/Shaders/Particle.fragmentshader");

    // Vertex shader
    GLuint CameraRight_worldspace_ID = glGetUniformLocation(programID, "CameraRight_worldspace");
    GLuint CameraUp_worldspace_ID = glGetUniformLocation(programID, "CameraUp_worldspace");
    GLuint ViewProjMatrixID = glGetUniformLocation(programID, "VP");

    // fragment shader
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");


    static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
    static GLubyte* g_particule_color_data = new GLubyte[MaxParticles * 4];

    for (int i = 0; i < MaxParticles; i++) {
        ParticlesContainer[i].life = -1.0f;
        ParticlesContainer[i].cameradistance = -1.0f;
    }



    GLuint Texture = loadDDS("../Glitter/objects/particle.DDS");

    // The VBO containing the 4 vertices of the particles.
    // Thanks to instancing, they will be shared by all particles.
    static const GLfloat g_vertex_buffer_data[] = {
         -0.5f, -0.5f, 0.0f,
          0.5f, -0.5f, 0.0f,
         -0.5f,  0.5f, 0.0f,
          0.5f,  0.5f, 0.0f,
    };
    GLuint billboard_vertex_buffer;
    glGenBuffers(1, &billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // The VBO containing the positions and sizes of the particles
    GLuint particles_position_buffer;
    glGenBuffers(1, &particles_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    // The VBO containing the colors of the particles
    GLuint particles_color_buffer;
    glGenBuffers(1, &particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);











    initWaterPart(&cubeShader,&waterShader,&quadShader);

    initModel(&ourShader);
    quadShader.use();

    shaderBlur.use();
    shaderBlur.setInt("image", 0);


    //glm::mat4 ViewProjectionMatrix = camera.GetViewMatrix() * projection;
    // render loop
    // -----------
    lastFrame= static_cast<float>(glfwGetTime());
    while (!glfwWindowShouldClose(window))
    {


        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // input

        processInput(window);
        animator.UpdateAnimation(deltaTime);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.4f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(.01f, .01f, .01f));	// it's a bit too big for our scene, so scale it down


        glm::mat4 projection = glm::perspective(3.1415f / 2.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 ProjectionMatrix = projection;
        glm::mat4 ViewMatrix = camera.GetViewMatrix();

        // We will need the camera's position in order to sort the particles
        // w.r.t the camera's distance.
        // There should be a getCameraPosition() function in common/controls.cpp, 
        // but this works too.
        glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);

        glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
        
        
    /*    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.5f, 0.2f, 0.3f, 1.0f);*/
        int newparticles = (int)(deltaTime * 10000.0);
        if (newparticles > (int)(0.016f * 10000.0))
            newparticles = (int)(0.016f * 10000.0);

        for (int i = 0; i < newparticles; i++) {
            int particleIndex = FindUnusedParticle();
            ParticlesContainer[particleIndex].life = 2.0f; // This particle will live 5 seconds.
            ParticlesContainer[particleIndex].pos = glm::vec3(0, 2.5f, 0);

            float spread = 1.5f;
            glm::vec3 maindir = glm::vec3(0.0f, 0.1f, 5.0f);
            // Very bad way to generate a random direction; 
            // See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
            // combined with some user-controlled parameters (main direction, spread, etc)
            glm::vec3 randomdir = glm::vec3(
                (rand() % 2000 - 1000.0f) / 1000.0f,
                (rand() % 2000 - 1000.0f) / 1000.0f,
                (rand() % 2000 - 1000.0f) / 1000.0f
            );

            ParticlesContainer[particleIndex].speed = maindir + randomdir * spread;


            // Very bad way to generate a random color
            ParticlesContainer[particleIndex].r = rand() % 256;
            ParticlesContainer[particleIndex].g = rand() % 256;
            ParticlesContainer[particleIndex].b = rand() % 256;
            ParticlesContainer[particleIndex].a = (rand() % 256) / 3;

            ParticlesContainer[particleIndex].size = (rand() % 1000) / 2000.0f + 0.1f;

        }



        // Simulate all particles
        int ParticlesCount = 0;
        for (int i = 0; i < MaxParticles; i++) {

            Particle& p = ParticlesContainer[i]; // shortcut

            if (p.life > 0.0f) {

                // Decrease life
                p.life -= deltaTime;
                if (p.life > 0.0f) {

                    // Simulate simple physics : gravity only, no collisions
                    p.speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float)deltaTime * 0.5f;
                    p.pos += p.speed * (float)deltaTime;
                    p.cameradistance = glm::length2(p.pos - camera.Position);
                    //ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

                    // Fill the GPU buffer
                    g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
                    g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
                    g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

                    g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

                    g_particule_color_data[4 * ParticlesCount + 0] = p.r;
                    g_particule_color_data[4 * ParticlesCount + 1] = p.g;
                    g_particule_color_data[4 * ParticlesCount + 2] = p.b;
                    g_particule_color_data[4 * ParticlesCount + 3] = p.a;

                }
                else {
                    // Particles that just died will be put at the end of the buffer in SortParticles();
                    p.cameradistance = -1.0f;
                }

                ParticlesCount++;

            }
        }

        SortParticles();


        //printf("%d ",ParticlesCount);


        // Update the buffers that OpenGL uses for rendering.
        // There are much more sophisticated means to stream data from the CPU to the GPU, 
        // but this is outside the scope of this tutorial.
        // http://www.opengl.org/wiki/Buffer_Object_Streaming

        glBindFramebuffer(GL_FRAMEBUFFER, particle_buffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
        glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
        glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);


        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Use our shader
        glUseProgram(programID);
        glBindVertexArray(VertexArrayID);
        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);

        // Same as the billboards tutorial
        glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
        glUniform3f(CameraUp_worldspace_ID, ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

        glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
        glVertexAttribPointer(
            0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // 2nd attribute buffer : positions of particles' centers
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        glVertexAttribPointer(
            1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            4,                                // size : x + y + z + size => 4
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        // 3rd attribute buffer : particles' colors
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        glVertexAttribPointer(
            2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            4,                                // size : r + g + b + a => 4
            GL_UNSIGNED_BYTE,                 // type
            GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        // These functions are specific to glDrawArrays*Instanced*.
        // The first parameter is the attribute buffer we're talking about.
        // The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
        // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
        glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
        glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
        glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

        // Draw the particules !
        // This draws many times a small triangle_strip (which looks like a quad).
        // This is equivalent to :
        // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
        // but faster.
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glBindBuffer(GL_FRAMEBUFFER, 0);


        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // don't forget to clear the stencil buffer!
        glStencilMask(0x00);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        ourShader.use();
        initModel(&ourShader);
        //
        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
         ourShader.setMat4("model", model);
        //
        ourModel.Draw(ourShader);
        ////泛光收尾函数
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);
        ourShaderSingle.use();
        initModel(&ourShaderSingle);
        //ourModel.Draw(ourShaderSingle);
        glBindVertexArray(0);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 1000;
        shaderBlur.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            shaderBlur.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);
        renderwater(&ourShader, &ourModel, &shaderBlur);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------

void initGL(){
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_SAMPLES, SAMPLES);


    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);
    //stbi_set_flip_vertically_on_load(true);
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
void initWaterPart(Shader* cubeShader, Shader* waterShader, Shader* quadShader) {
    for (size_t i = 0; i < 5; i++) {
        cube_base[i].Init(cubeShader);
        cube_base[i].SetColor(glm::vec3(1.0f - 0.1f * i, 1.0f - 0.1f * i, 1.0f - 0.1f * i));
        Drawable_list.push_back(&cube_base[i]);
    }
    cube_decoration[0].Init(cubeShader);
    cube_decoration[0].SetColor(glm::vec3(0.6f, 1.0f, 0.6f));
    cube_decoration[1].Init(cubeShader);
    cube_decoration[1].SetColor(glm::vec3(1.0f, 0.6f, 0.6f));
    Drawable_list.push_back(&cube_decoration[0]);
    Drawable_list.push_back(&cube_decoration[1]);

    water.Init(waterShader);
    Drawable_list.push_back(&water);

    cube_base_trans[0] = glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, 0.0f, 0.0f));
    cube_base_trans[0] = glm::scale(cube_base_trans[0], glm::vec3(5.0f, 2.5f, 20.0f));
    cube_base_trans[1] = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, -15.0f));
    cube_base_trans[1] = glm::scale(cube_base_trans[1], glm::vec3(15.0f, 2.5f, 5.0f));
    cube_base_trans[2] = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, 15.0f));
    cube_base_trans[2] = glm::scale(cube_base_trans[2], glm::vec3(15.0f, 2.5f, 5.0f));
    cube_base_trans[3] = glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f, 0.0f, 0.0f));
    cube_base_trans[3] = glm::scale(cube_base_trans[3], glm::vec3(5.0f, 2.5f, 10.0f));
    cube_base_trans[4] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.5f, 0.0f));
    cube_base_trans[4] = glm::scale(cube_base_trans[4], glm::vec3(20.0f, 0.5f, 20.0f));

    cube_deco_trans[0] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, -4.0f));
    cube_deco_trans[0] = glm::scale(cube_deco_trans[0], glm::vec3(2.5f, 2.5f, 2.5f));
    cube_deco_trans[1] = glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f, 5.0f, 4.0f));
    cube_deco_trans[1] = glm::scale(cube_deco_trans[1], glm::vec3(2.5f, 2.5f, 2.5f));

    water_trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
    water_trans = glm::scale(water_trans, glm::vec3(20.0f, 20.0f, 20.0f));
    glClearColor(0.3f, 0.6f, 1.0f, 1.0f);
    //framebuffers
    framebuffer_refraction = new Framebuffer();
    framebuffer_refraction->Init(SCR_WIDTH, SCR_HEIGHT, false);

    framebuffer_reflection = new Framebuffer();
    framebuffer_reflection->Init(SCR_WIDTH, SCR_HEIGHT, false);

    depth_framebuffer = new DepthFramebuffer();
    depth_framebuffer->Init(SCR_WIDTH, SCR_HEIGHT);
    depth_framebuffer->SetPerspective(projection);

    framebuffer = new Framebuffer();
    GLuint tex_fb = framebuffer->Init(SCR_WIDTH, SCR_HEIGHT, true);
    GLuint tex_noise = loadTexture("../Glitter/objects/noise.png");
    water.SetGussPingPongTexture(pingpongColorbuffers[1]);
    water.SetGussPingPong_2Texture(colorBuffers[0]);
    water.SetRefractTexture(framebuffer_refraction->get_texture());
    water.SetReflectTexture(framebuffer_reflection->get_texture());
    //TODO 把另一个buffer给加进来
    water.set_texture_refraction_depth(depth_framebuffer->GetTexId());
    water.SetNoiseTexture(tex_noise);
    water.SetParticleTexture(textureColorbuffer);
    quad_screen.Init(tex_fb, SCR_WIDTH, SCR_HEIGHT, quadShader->ID);

    depth_framebuffer->SetCamera(&camera);

    //clip coord to tell shader not to draw anything over the water
    for (size_t i = 0; i < Drawable_list.size(); i++) {
        Drawable_list[i]->SetClipCoord(0, 1, 0, -2);
    }

    light_mode_selected = 1;
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

void renderwater(Shader* ourShader, Model* ourModel, Shader* shaderBlur) {
    water.set_effect(water_effect);
    water.SetGussPingPongTexture(pingpongColorbuffers[1]);
    water.SetGussPingPong_2Texture(colorBuffers[0]);
    for (size_t i = 0; i < 5; i++) {
        cube_base[i].SetMVPMat(cube_base_trans[i], camera.GetViewMatrix(), projection);
    }

    for (size_t i = 0; i < 2; i++) {
        cube_decoration[i].SetMVPMat(cube_deco_trans[i], camera.GetViewMatrix(), projection);
    }

    water.SetMVPMat(water_trans, camera.GetViewMatrix(), projection);
    water.SetTime(glfwGetTime()); //time for the movement of waves

    for (size_t i = 0; i < Drawable_list.size(); i++) {
        Drawable_list[i]->SetLightPos(light_pos);

        Drawable_list[i]->SetCameraPos(camera.Position);
        Drawable_list[i]->SetCameraDir(camera.Front);

        Drawable_list[i]->set_window_dim(SCR_WIDTH, SCR_HEIGHT);

        Drawable_list[i]->set_shadow_buffer_texture_size(SCR_WIDTH, SCR_HEIGHT);
        Drawable_list[i]->set_shadow_buffer_texture(depth_framebuffer->GetTexId());
        Drawable_list[i]->SetShadow(depth_framebuffer->GetShadow());
    }
    water.SetEnabled(false); //dont display water in refraction

    framebuffer_refraction->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    for (size_t i = 0; i < Drawable_list.size(); i++) {
        Drawable_list[i]->Draw();
    }

    framebuffer_refraction->unbind();

    depth_framebuffer->DrawFB(&Drawable_list);

    water.SetEnabled(true);

    //==============REFLEXION STEP
    framebuffer_reflection->bind();

    glEnable(GL_CLIP_DISTANCE0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    renderQuad();
    //use the different view matrix which is from under the water
    for (size_t i = 0; i < 5; i++) {
        cube_base[i].SetMVPMat(cube_base_trans[i], camera.GetReflectMatrix(2.0f), projection);
    }

    for (size_t i = 0; i < 2; i++) {
        cube_decoration[i].SetMVPMat(cube_deco_trans[i], camera.GetReflectMatrix(2), projection);
    }

    water.SetMVPMat(water_trans, camera.GetReflectMatrix(2), projection);

    for (size_t i = 0; i < Drawable_list.size(); i++) {
        Drawable_list[i]->Draw();
    }

    glDisable(GL_CLIP_DISTANCE0);
    framebuffer_reflection->unbind();

    //sets back the normal view matrix
    for (size_t i = 0; i < 5; i++) {
        cube_base[i].SetMVPMat(cube_base_trans[i], camera.GetViewMatrix(), projection);
    }

    for (size_t i = 0; i < 2; i++) {
        cube_decoration[i].SetMVPMat(cube_deco_trans[i], camera.GetViewMatrix(), projection);
    }

    water.SetMVPMat(water_trans, camera.GetViewMatrix(), projection);
    //===============FINAL STEP
    //print to the real, color, multisample frambuffer
    framebuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //ourShader->use();
    //initModel(ourShader);
    //ourModel->Draw(*ourShader);
    for (size_t i = 0; i < Drawable_list.size(); i++) {
        Drawable_list[i]->Draw();
    }

    framebuffer->unbind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    quad_screen.set_model_before_texture(colorBuffers[0]);
    quad_screen.set_model_texture(pingpongColorbuffers[1]);
    quad_screen.set_model_particle_texture(textureColorbuffer);
    quad_screen.draw(effect_select);

    if (light_mode_selected == 0) {
        light_pos = glm::vec3(10, 15, -10);
    }
    else if (light_mode_selected == 1) {
        light_pos = glm::vec3(16.0 * cos(glfwGetTime() / 2), 8, 16.0 * sin(glfwGetTime() / 2));
    }
    else if (light_mode_selected == 2) {
        light_pos = glm::vec3(30, 30, 0);
    }
    else if (light_mode_selected == 3) {
        light_pos = glm::vec3(30, 30, 30);
    }

}

void initModel(Shader* ourShader) {
    ourShader->use();
    // directional light
    ourShader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    ourShader->setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    ourShader->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    ourShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // point light 1
    ourShader->setVec3("pointLights[0].position", pointLightPositions[0]);
    ourShader->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    ourShader->setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    ourShader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    ourShader->setFloat("pointLights[0].constant", 1.0f);
    ourShader->setFloat("pointLights[0].linear", 0.09f);
    ourShader->setFloat("pointLights[0].quadratic", 0.032f);
    // point light 2
    ourShader->setVec3("pointLights[1].position", pointLightPositions[1]);
    ourShader->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    ourShader->setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
    ourShader->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    ourShader->setFloat("pointLights[1].constant", 1.0f);
    ourShader->setFloat("pointLights[1].linear", 0.09f);
    ourShader->setFloat("pointLights[1].quadratic", 0.032f);
    // point light 3
    ourShader->setVec3("pointLights[2].position", pointLightPositions[2]);
    ourShader->setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
    ourShader->setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
    ourShader->setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
    ourShader->setFloat("pointLights[2].constant", 1.0f);
    ourShader->setFloat("pointLights[2].linear", 0.09f);
    ourShader->setFloat("pointLights[2].quadratic", 0.032f);
    // point light 4
    ourShader->setVec3("pointLights[3].position", pointLightPositions[3]);
    ourShader->setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
    ourShader->setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
    ourShader->setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
    ourShader->setFloat("pointLights[3].constant", 1.0f);
    ourShader->setFloat("pointLights[3].linear", 0.09f);
    ourShader->setFloat("pointLights[3].quadratic", 0.032f);
    // spotLight
    ourShader->setVec3("spotLight.position", camera.Position);
    ourShader->setVec3("spotLight.direction", camera.Front);
    ourShader->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    ourShader->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    ourShader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    ourShader->setFloat("spotLight.constant", 1.0f);
    ourShader->setFloat("spotLight.linear", 0.09f);
    ourShader->setFloat("spotLight.quadratic", 0.032f);
    ourShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    ourShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));


    ourShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
    // material properties
    ourShader->setFloat("material.shininess", 64.0f);

    glm::mat4 view = camera.GetViewMatrix();

    ourShader->setMat4("projection", projection);
    ourShader->setMat4("view", camera.GetViewMatrix());



    // draw floor as normal, but don't write the floor to the stencil buffer, we only care about the containers. We set its mask to 0x00 to not write to the stencil buffer.

    // render the loaded model
    /*
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f));	// it's a bit too big for our scene, so scale it down

    model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
    ourShader->setMat4("model", model);*/
}

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