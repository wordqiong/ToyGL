// Local Headers

#include "glitter.hpp"
#include <shader.h>

#include <model.h>
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(std::vector<std::string> faces);
unsigned int loadTexture(char const* path);
void initGL();
void initWaterPart(Shader* cubeShader,Shader* waterShader,Shader* quadShader);
void renderwater();
//window
GLFWwindow* window;
QuadScreen quad_screen;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.5f, 2.0f));
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
int main()
{
    initGL();
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
    
    //watershader
    Shader cubeShader("../Glitter/cube.vs", "../Glitter/cube.fs");
    Shader waterShader("../Glitter/water_vshader.glsl", "../Glitter/water_fshader.glsl");
    Shader quadShader("../Glitter/quad_screen_vshader.glsl", "../Glitter/quad_screen_fshader.glsl");
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


    initWaterPart(&cubeShader,&waterShader,&quadShader);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // input
        processInput(window);

        renderwater();

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

void initGL() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_SAMPLES, SAMPLES);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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
    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
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

    water.SetRefractTexture(framebuffer_refraction->get_texture());
    water.SetReflectTexture(framebuffer_reflection->get_texture());
    water.set_texture_refraction_depth(depth_framebuffer->GetTexId());
    water.SetNoiseTexture(tex_noise);

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

void renderwater() {
    water.set_effect(water_effect);
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (size_t i = 0; i < Drawable_list.size(); i++) {
        Drawable_list[i]->Draw();
    }

    framebuffer_refraction->unbind();

    depth_framebuffer->DrawFB(&Drawable_list);

    water.SetEnabled(true);

    //==============REFLEXION STEP
    framebuffer_reflection->bind();
    glEnable(GL_CLIP_DISTANCE0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (size_t i = 0; i < Drawable_list.size(); i++) {
        Drawable_list[i]->Draw();
    }

    framebuffer->unbind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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