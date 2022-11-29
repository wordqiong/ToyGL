#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// two triangles that should take all the screen space to draw the framebuffer
class QuadScreen{
protected:
  GLuint _vao;
  GLuint _sid;
  GLuint _vbo;
  GLuint _tex;
  GLuint _ao_tex;
  GLuint _depth_tex;
  GLuint _width;
  GLuint _height;

public:

  //texture is the framebuffer
  void Init(GLuint texture, GLuint screen_width, GLuint screen_height, GLuint sid){
    this->_width = screen_width;
    this->_height = screen_height;

    _sid = sid;
    if(_sid == 0)
      exit(-1);

    glUseProgram(_sid);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    const GLfloat vpoint[] = { 
      -1.0f, -1.0f, 0.0f,
      +1.0f, -1.0f, 0.0f,
      -1.0f, +1.0f, 0.0f,
      +1.0f, +1.0f, 0.0f 
    };

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

    GLuint vpoint_id = glGetAttribLocation(_sid, "vpoint");
    glEnableVertexAttribArray(vpoint_id);
    glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    const GLfloat vtexcoord[] = { 
      /*V1*/ 0.0f, 0.0f,
      /*V2*/ 1.0f, 0.0f,
      /*V3*/ 0.0f, 1.0f,
      /*V4*/ 1.0f, 1.0f
    };

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

    GLuint vtexcoord_id = glGetAttribLocation(_sid, "vtexcoord");
    glEnableVertexAttribArray(vtexcoord_id);
    glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    this->_tex = texture;

    glBindVertexArray(0);
    glUseProgram(0);
  }

  void load_texture(GLuint texture){
    this->_tex = texture;
  }

  void set_ao_texture(GLuint ao_tex){
    this->_ao_tex = ao_tex;
  }

  void set_depth_texture(GLuint depth_tex){
    this->_depth_tex = depth_tex;
  }

  void cleanup(){
    /// TODO
  }

  void draw(unsigned int effect_select){
    glUseProgram(_sid);
    glBindVertexArray(_vao);

    glViewport(0, 0, _width, _height);

    glUniform1ui( glGetUniformLocation(_sid, "effect_select"), effect_select);
    glUniform1f( glGetUniformLocation(_sid, "tex_width"), _width);
    glUniform1f( glGetUniformLocation(_sid, "tex_height"), _height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _tex);
    GLuint tex_id = glGetUniformLocation(_sid, "tex");
    glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _ao_tex);
    tex_id = glGetUniformLocation(_sid, "ao_tex");
    glUniform1i(tex_id, 1 /*GL_TEXTURE1*/);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _depth_tex);
    tex_id = glGetUniformLocation(_sid, "depth_tex");
    glUniform1i(tex_id, 2 /*GL_TEXTURE1*/);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
  }

};
