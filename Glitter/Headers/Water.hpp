#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#include "../include/common.h"
#include <shader.h>
#include <Texture.hpp>
#include <Drawable.hpp>

class Water : public Drawable{
protected:
   GLuint _vao;
   GLuint _vbo;
   GLuint _vbo_idx;
   GLuint _vbo_tex;
   GLuint _vbo_sur_norm;
   GLuint _tex;
   GLuint _tex1;
   GLuint _tex_refraction;
   GLuint _tex_reflection;
   GLuint _tex_depth_refraction;
   GLuint _tex_noise;
   GLuint _tex_pingpong;
   GLuint _tex_pingpong_2;
   GLuint _tex_particle;
   GLuint _effect;

   float _time;

   Shader* _waterShader;

public:
  void Init(Shader* waterShader){
    _waterShader = waterShader;
    _sid = _waterShader->ID;
    if(_sid == 0){
      std::cerr << "create water shader create failed" << std::endl;
      exit(-1);
    } 

    this->_effect = 0;

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    GLfloat vpoint[] = {
        -1.0f, 0.0f,  1.0f, // 0 bottom left
         1.0f, 0.0f,  1.0f, // 1 bottom right
         1.0f, 0.0f, -1.0f, // 2 top right
        -1.0f, 0.0f, -1.0f, // 3 top left
    };

    GLuint vpoint_index[] = {
        0, 1, 2,
        0, 2, 3
    };

    GLfloat per_surface_normals[] = {  
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
    };

    _time = 0.0f;

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

    GLuint vpoint_id = glGetAttribLocation(_sid, "position");
    glEnableVertexAttribArray(vpoint_id);
    glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glGenBuffers(1, &_vbo_sur_norm);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
    glBufferData(GL_ARRAY_BUFFER, sizeof(per_surface_normals), per_surface_normals, GL_STATIC_DRAW);

    GLuint id_pos = glGetAttribLocation(_sid, "surface_normal");
    glEnableVertexAttribArray(id_pos);
    glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glGenBuffers(1, &_vbo_idx);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vpoint_index), vpoint_index, GL_STATIC_DRAW);

    //texture coord definition
    const GLfloat vtexcoord[] = { 
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f
    };

    glGenBuffers(1, &_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

    GLuint vtexcoord_id = glGetAttribLocation(_sid, "uv");
    glEnableVertexAttribArray(vtexcoord_id);
    glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
  }

  void Init(GLuint sid, GLuint vao){
    _sid = sid;
    _vao = vao;
  }

  void SetTexture(const WTexture *tex){
    if(_tex != 0){
      //delete current texture
      glDeleteTextures(1, &_tex);
      _tex = 0;
    }

    //  texture data definition
    glGenTextures(1, &_tex);
    glBindTexture(GL_TEXTURE_2D, _tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, tex->get_width(), tex->get_height(), 0, GL_RED, GL_FLOAT, tex->get_tex_data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    _waterShader->setInt("tex", 0);
  }

  void SetTexture(GLuint tex){
    _tex = tex;
  }

  void SetTexture1(GLuint tex1){
    _tex1 = tex1;
  }

  void SetTime(float time){
    _time = time;
  }

  void SetRefractTexture(GLuint tex) {
      _tex_refraction = tex;
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex_refraction);
      _waterShader->setInt("tex_refraction", 0);
      glBindTexture(GL_TEXTURE_2D, 0);
  }
  void SetGussPingPongTexture(GLuint tex) {
      _tex_pingpong = tex;
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, _tex_pingpong);
      _waterShader->setInt("tex_pingpong", 4);
      glBindTexture(GL_TEXTURE_2D, 0);
  }
  void SetGussPingPong_2Texture(GLuint tex) {
      _tex_pingpong_2 = tex;
      glActiveTexture(GL_TEXTURE5);
      glBindTexture(GL_TEXTURE_2D, _tex_pingpong_2);
      _waterShader->setInt("tex_pingpong_2", 5);
      glBindTexture(GL_TEXTURE_2D, 0);
  }

  void SetParticleTexture(GLuint tex) {
      _tex_particle = tex;
      glActiveTexture(GL_TEXTURE5);
      glBindTexture(GL_TEXTURE_2D, _tex_particle);
      _waterShader->setInt("tex_particle", 6);
      glBindTexture(GL_TEXTURE_2D, 0);
  }


  void SetReflectTexture(GLuint tex){
    _tex_reflection = tex;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _tex_reflection);
    _waterShader->setInt("tex_reflection", 1);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void set_texture_refraction_depth(GLuint tex){
    _tex_depth_refraction = tex;
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _tex_depth_refraction);
    _waterShader->setInt("tex_depth_refraction", 2);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void SetNoiseTexture(GLuint tex){
    _tex_noise = tex;
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, _tex_noise);
    _waterShader->setInt("tex_noise", 3);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void set_effect(unsigned int effect){
    this->_effect = effect;
  }

  void Draw(){
    glUseProgram(_sid);
    glBindVertexArray(_vao);

    _waterShader->setVec3("light_position", this->_light_pos);
    _waterShader->setVec3("camera_position", this->_camera_pos);
    _waterShader->setVec3("camera_direction", this->_camera_dir);

//      glUniform1f( glGetUniformLocation(_sid, "time"), this->_time);
    _waterShader->setFloat("time", this->_time);
    _waterShader->setInt("effect", this->_effect);

    _waterShader->setMat4("model", this->_model);
    _waterShader->setMat4("view", this->_view);
    _waterShader->setMat4("projection", this->_projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _tex_refraction);
    _waterShader->setInt("tex_refraction", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _tex_reflection);
    _waterShader->setInt("tex_reflection", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _tex_depth_refraction);
    _waterShader->setInt("tex_depth_refraction", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, _tex_noise);
    _waterShader->setInt("tex_noise", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, _tex_pingpong);
    _waterShader->setInt("tex_pingpong", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, _tex_pingpong_2);
    _waterShader->setInt("tex_pingpong_2", 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, _tex_particle);
    _waterShader->setInt("tex_particle", 6);

    if(_enabled){
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    //  unbind texture, shader program, vertex array
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glBindVertexArray(0);
  }

  void cleanup(){
    glDeleteBuffers(1, &_vbo);
    glDeleteVertexArrays(1, &_vao);
    glDeleteProgram(_waterShader->ID);
  }
};
