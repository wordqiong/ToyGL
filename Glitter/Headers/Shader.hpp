/* Copyright (c) 2019 by Stan Fortoński */

#ifndef SHADER_HPP
#define SHADER_HPP 1
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include "../include/glad/glad.h"

class Shader_water
{
  unsigned shaderId;
  GLenum type;

  std::string getSource(const std::string & fileName);
  std::string getCompileMessageErrorAndClear();
public:
	Shader_water(const std::string & fileName, GLenum t);
  void clear() const{glDeleteShader(shaderId);}

  unsigned getId() const {return shaderId;}
  GLenum getType() const {return type;}
};
#endif
