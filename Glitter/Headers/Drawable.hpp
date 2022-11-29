#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//  interface for a drawable element such as a cube or a sphere
class Drawable{
protected:
  GLuint _sid;  //  shader id

  glm::mat4 _model;
  glm::mat4 _view;
  glm::mat4 _projection;
  glm::mat4 _shadow;

  glm::vec3 _camera_pos;
  glm::vec3 _camera_dir;
  glm::vec3 _sun_dir;
  glm::vec3 _sun_col;
  glm::vec3 _light_pos;

  glm::vec4 _clip_coord;

  bool _has_shadow_buffer;
  bool _enabled;

  GLuint _shadow_tex_id;

  GLuint _shadow_mapping_effect;
  GLuint _window_width;
  GLuint _window_height;
  GLuint _shadow_buffer_texture_width;
  GLuint _shadow_buffer_texture_height;

public:
  Drawable(){
    _has_shadow_buffer = false;
    _shadow_mapping_effect = 0;
    _window_width = 0;
    _window_height = 0;
    _enabled = true;
  }

  virtual void SetModel(glm::mat4 model){
    this->_model = model;
  }

  virtual void SetView(glm::mat4 view){
    this->_view = view;
  }

  virtual void SetProjection(glm::mat4 projection){
    this->_projection = projection;
  }

  virtual void SetMVPMat(glm::mat4 model, glm::mat4 view, glm::mat4 projection){
    SetModel(model);
    SetView(view);
    SetProjection(projection);
  }

  virtual void SetLightPos(glm::vec3 light_pos){
    this->_light_pos = light_pos;
  }

  virtual void SetCameraPos(glm::vec3 camera_pos){
    this->_camera_pos = camera_pos;
  }

  virtual void SetCameraDir(glm::vec3 camera_dir){
    this->_camera_dir = camera_dir;
  }

  virtual void SetShadow(glm::mat4 shadow){
    this->_shadow = shadow;
  }

  virtual void set_shadow_buffer_texture(GLuint sh_tex){
    _has_shadow_buffer = true;
    _shadow_tex_id = sh_tex;
  }

  virtual void set_shadow_buffer_texture_size(unsigned int shadow_buffer_tex_size){
    _shadow_buffer_texture_width = shadow_buffer_tex_size;
    _shadow_buffer_texture_height = shadow_buffer_tex_size;
  }

  virtual void set_shadow_buffer_texture_size(unsigned int shadow_buffer_tex_width, unsigned int shadow_buffer_tex_height){
    _shadow_buffer_texture_width = shadow_buffer_tex_width;
    _shadow_buffer_texture_height = shadow_buffer_tex_height;
  }

  virtual void set_shadow_mapping_effect(unsigned int shadow_mapping_effect){
    _shadow_mapping_effect = shadow_mapping_effect;
  }

  virtual void set_window_dim(unsigned int win_width, unsigned int win_height){
    _window_width = win_width;
    _window_height = win_height;
  }

  virtual void set_shader(GLuint shader_pid){
    _sid = shader_pid;
  }

  virtual GLuint get_shader(){
    return _sid;
  }

  virtual void SetClipCoord(float x, float y, float z, float d){
    _clip_coord.x = x;
    _clip_coord.y = y;
    _clip_coord.z = z;
    _clip_coord.w = d;
  }

  virtual void SetEnabled(bool enabled){
    _enabled = enabled;
  }

  virtual void set_sun_dir(glm::vec3 sun_dir){
    _sun_dir = sun_dir;
  }

  virtual void set_sun_col(glm::vec3 sun_col){
    _sun_col = sun_col;
  }

  virtual void Draw() = 0;
};

