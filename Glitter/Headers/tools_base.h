#ifndef TOOLS_BASE
#define TOOLS_BASE
#include <camera.h>
// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//设置一些基本的初始化窗口的函数 基本完成camera，Windows的设置
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;

// camera
extern Camera camera;
// timing
extern float deltaTime;
extern float lastFrame;
extern float lastX;
extern float lastY;
extern bool firstMouse;

void processInput(GLFWwindow* window);

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
#endif