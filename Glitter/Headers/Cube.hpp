#pragma once 

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#include "../include/common.h"
#include <shader.h>
#include "Drawable.hpp"

class Cube : public Drawable{

protected:
  GLuint _vao;
  GLuint _vbo_pos;
  GLuint _vbo_sur_norm;
  GLuint _num_indices;
  GLuint _num_vertices;

  glm::vec3 _color;

  Shader* _cubeShader;

public:
  void Init(Shader* s){
    _cubeShader = s;
    _sid = s->ID;
    if(_sid == 0){
      std::cerr << "create water shader create failed" << std::endl;
      exit(-1);
    } 

    glUseProgram(_sid);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    GLfloat indexed_position[] = { 
      -1.0f, -1.0f,  1.0f,
       1.0f, -1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f
    };

    GLuint index[] = {
      0, 1, 2,//front
      0, 2, 3,
      1, 5, 6,//right
      1, 6, 2,
      5, 4, 7,//back
      5, 7, 6,
      4, 0, 3,//left
      4, 3, 7,
      3, 2, 6,//top
      3, 6, 7,
      1, 0, 4,//bottom
      1, 4, 5
    };

    //we need to generate positions one by one in order to have per surface vertex
    GLfloat position[36*3];
    for (size_t i = 0; i < 36; i++) {
      position[i*3+0] = indexed_position[index[i]*3+0];
      position[i*3+1] = indexed_position[index[i]*3+1];
      position[i*3+2] = indexed_position[index[i]*3+2];
    }

    GLfloat per_surface_normals[] = {
      0,  0,  1,  0,  0,  1,  0,  0,  1,
      0,  0,  1,  0,  0,  1,  0,  0,  1,
      1,  0,  0,  1,  0,  0,  1,  0,  0,
      1,  0,  0,  1,  0,  0,  1,  0,  0,
      0,  0, -1,  0,  0, -1,  0,  0, -1,
      0,  0, -1,  0,  0, -1,  0,  0, -1,
     -1,  0,  0, -1,  0,  0, -1,  0,  0,
     -1,  0,  0, -1,  0,  0, -1,  0,  0,
      0,  1,  0,  0,  1,  0,  0,  1,  0,
      0,  1,  0,  0,  1,  0,  0,  1,  0,
      0, -1,  0,  0, -1,  0,  0, -1,  0,
      0, -1,  0,  0, -1,  0,  0, -1,  0
    };

    glGenBuffers(1, &_vbo_pos);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

    GLuint id_pos = glGetAttribLocation(_sid, "position");
    glEnableVertexAttribArray(id_pos);
    glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    _num_vertices = sizeof(position)/sizeof(GLfloat);
    _num_indices = sizeof(index)/sizeof(GLuint);

    glGenBuffers(1, &_vbo_sur_norm);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
    glBufferData(GL_ARRAY_BUFFER, sizeof(per_surface_normals), per_surface_normals, GL_STATIC_DRAW);

    id_pos = glGetAttribLocation(_sid, "surface_normal");
    glEnableVertexAttribArray(id_pos);
    glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    _color = glm::vec3(1.0f, 1.0f, 1.0f);

    glBindVertexArray(0);
  }

  void Draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection){
    glUseProgram(_sid);
    glBindVertexArray(_vao);

    _cubeShader->setMat4("model", model);
    _cubeShader->setMat4("view", view);
    _cubeShader->setMat4("projection", projection);

    glDrawArrays(GL_TRIANGLES, 0, _num_vertices);

    //  unbind shader program and vao
    glUseProgram(0);
    glBindVertexArray(0);
  }

  void SetColor(glm::vec3 rgb){
    _color = rgb; 
  }

  void Draw(){
    glUseProgram(_sid);
    glBindVertexArray(_vao);

    _cubeShader->setVec3("light_position", _light_pos);
    _cubeShader->setVec3("camera_position", _camera_pos);
    _cubeShader->setVec3("shape_color", _color);
    _cubeShader->setVec4("clip_coord", _clip_coord);

    _cubeShader->setInt("shadow_mapping_effect", _shadow_mapping_effect);
    _cubeShader->setInt("shadow_buffer_tex_size", _shadow_buffer_texture_width);
    _cubeShader->setInt("window_width", _window_width);
    _cubeShader->setInt("window_height", _window_height);

    _cubeShader->setVec3("sun_dir", _sun_dir);
    _cubeShader->setVec3("sun_col", _sun_col);


    if(_has_shadow_buffer){
      _cubeShader->setMat4("shadow_matrix", _shadow);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, _shadow_tex_id);

      _cubeShader->setInt("shadow_buffer_tex", 1);
    }

    Draw(_model, _view, _projection);

    if(_has_shadow_buffer){
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glUseProgram(0);
    glBindVertexArray(0);
  }

  void cleanup(){
    glDeleteBuffers(1, &_vbo_pos);
    glDeleteBuffers(1, &_vbo_sur_norm);
    glDeleteVertexArrays(1, &_vao);
    glDeleteProgram(_sid);
  }
};
