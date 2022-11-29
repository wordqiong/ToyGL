#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <camera.h>
//#include "camera.h"
#include <Drawable.hpp>
#include <iostream>

const float NEARPLANE = 1.0f; 
const float FARPLANE  = 1000.0f; 
const int   DEFAULT_PERSPECTIVE = 3.14159f / 1.6f;

//  framebuffer that keeps the depth information in its texture
class DepthFramebuffer{
public:

  void Init(unsigned tex_width, unsigned tex_height, float perspective_angle = DEFAULT_PERSPECTIVE) {
    this->tex_width = tex_width;
    this->tex_height = tex_height;

    this->light_pos = glm::vec3(0.0f,0.0f,0.0f); 

    //  The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers(1, &depth_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

    //  Depth texture. Slower than a depth buffer, but you can sample it later in your shader
    glGenTextures(1, &depth_tex_id);
    glBindTexture(GL_TEXTURE_2D, depth_tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT32, tex_width, tex_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    //  set filter attr
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //  set wrap attr
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex_id, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cerr << "!!!ERROR: depth Framebuffer not OK :(" << std::endl;

    //  Disable color rendering as there are no color attachments
    glDrawBuffer(GL_NONE);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //  default perspective matrix
    SetPerspective(glm::perspective(perspective_angle, (float)tex_width/(float)tex_height, 1.0f, 1000.0f));
  }

  void SetLightPos(glm::vec3 LightPos){
    light_pos = LightPos;
  }

  void SetLightDir(glm::vec3 Lightdir){
    light_dir = Lightdir; 
  }

  void SetCamera(Camera* cam){
    this->pcamera = cam;
  }

  glm::mat4 GetShadow(){
    glm::mat4 bias_matrix(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0
    );
    return bias_matrix*projection*view;
  }

  GLuint GetTexId(){
    return depth_tex_id;
  }

  void SetPerspective(glm::mat4 perspective_mat){
    projection = perspective_mat;
  }

  glm::mat4 GetDepthView(){
    if(pcamera == NULL){
      glm::vec3 eye(light_pos);
      glm::vec3 center(light_dir);
      glm::vec3 up(0.0f, 1.0f, 0.0f);

      return glm::lookAt(eye, center, up);
    }
    else{
      return this->pcamera->GetViewMatrix();
      //return this->pcamera->getMatrix();
    }
  }

  glm::mat4 GetDepthProjection(){
    return projection;
  }

  void DrawFB(std::vector<Drawable*> *lst_drawable){
    glm::vec3 eye(light_pos);
    glm::vec3 center(light_dir);
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    glm::mat4 view_mat = glm::lookAt(eye, center, up);

    if(this->pcamera != NULL){
        view_mat = pcamera->GetViewMatrix();
        //view_mat = pcamera->getMatrix();
    }

    glm::mat4x4 projection_mat = GetDepthProjection();

    glClearDepth(1.0f);

    // Bind the "depth only" FBO and set the viewport to the size
    // of the depth texture
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
    glViewport(0, 0, tex_width, tex_height);

    glClear(GL_DEPTH_BUFFER_BIT);

    // Enable polygon offset to resolve depth-fighting isuses
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    // Draw from the light’s point of view
    for (size_t i = 0; i < lst_drawable->size(); i++) {
        //update the view and projection matrices
        lst_drawable->at(i)->SetView(view_mat);
        lst_drawable->at(i)->SetProjection(projection_mat);

        lst_drawable->at(i)->Draw();
    }

    glDisable(GL_POLYGON_OFFSET_FILL);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

protected:
  unsigned int depth_fbo;
  unsigned int depth_tex_id;
  unsigned int tex_width;
  unsigned int tex_height;

  glm::vec3 light_pos;
  glm::vec3 light_dir;

  glm::mat4 view;
  glm::mat4 projection;

  Camera* pcamera;
};

