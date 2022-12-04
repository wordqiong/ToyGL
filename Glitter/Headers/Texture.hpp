#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

class WTexture{
protected:
  GLuint tex_id;
  GLuint tex_width;
  GLuint tex_height;
  unsigned char *tex_data;

public:
  WTexture(){
    tex_data = NULL;
  }

  ~WTexture(){
    if(tex_data != NULL){
      stbi_image_free(tex_data);
    }
  }

  bool loadTexture(char const * path)
  {
    glGenTextures(1, &tex_id);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
      GLenum format;
      if (nrComponents == 1)
          format = GL_RED;
      else if (nrComponents == 3)
          format = GL_RGB;
      else if (nrComponents == 4)
          format = GL_RGBA;

      //  set texture member
      tex_width = width;
      tex_height = height;
      tex_data = data;

      glBindTexture(GL_TEXTURE_2D, tex_id);
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      return true;
    }
    else
    {
      std::cout << "Texture failed to load at path: " << path << std::endl;

      return false;
    }
  }

  //return the data of the image in bytes
  unsigned char *get_tex_data() const {
    return tex_data;
  }

  int get_width() const{
    return tex_width;
  }

  int get_height() const{
    return tex_height;
  }

};

#endif
