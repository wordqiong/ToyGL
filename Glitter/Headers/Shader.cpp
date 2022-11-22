/* Copyright (c) 2019 by Stan Fortoński */

#include "Shader.hpp"

void checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
Shader_water::Shader_water(const std::string & fileName, GLenum t): type(t)
{
  // 1. retrieve the source code from filePath
  std::string ShaderCode;
  std::ifstream ShaderFile;
  // ensure ifstream objects can throw exceptions:
  ShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try
  {
      // open files
      ShaderFile.open(fileName);
      std::stringstream ShaderStream;
      // read file's buffer contents into streams
      ShaderStream << ShaderFile.rdbuf();
      // close file handlers
      ShaderFile.close();
      // convert stream into string
      ShaderCode = ShaderStream.str();
  }
  catch (std::ifstream::failure& e)
  {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
  }
  const char* sShaderCode = ShaderCode.c_str();
  // 2. compile shaders
  shaderId = glCreateShader(type);
  glShaderSource(shaderId, 1, &sShaderCode, NULL);
  glCompileShader(shaderId);
  checkCompileErrors(shaderId, "Shader");
}

std::string Shader_water::getSource(const std::string & fileName) 
{
  std::ifstream file(fileName, std::ios::binary);
  if (!file.is_open())
    throw std::runtime_error("Can\'t open shader file: "+fileName+".");

  std::stringstream stream;
  stream<<file.rdbuf();

  file.clear();
  file.close();
  return stream.str();
}

std::string Shader_water::getCompileMessageErrorAndClear()
{
  int length;
  glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
  char* message = new char[length];

  glGetShaderInfoLog(shaderId, length, &length, message);
  glDeleteShader(shaderId);

  std::string finalMess = message;
  delete [] message;
  return finalMess;
}
