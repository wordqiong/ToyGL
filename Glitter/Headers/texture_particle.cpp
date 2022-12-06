#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glad/glad.h"
#include <iostream>

#include "texture_particle.h"

std::map<std::string, Par_Texture*> Par_Texture::loadedTextures;

Par_Texture::Par_Texture() :
	texture(0),
	channels(0),
	width(0),
	height(0) {

}

Par_Texture::Par_Texture(const std::string& path) {
	this->loadFromFile(path);
}

void Par_Texture::loadFromFile(const std::string& path) {
	if (loadedTextures.count(path) > 0) {
		this->texture = loadedTextures[path]->texture;
		this->width = loadedTextures[path]->width;
		this->height = loadedTextures[path]->height;
		this->channels = loadedTextures[path]->channels;
		return;
	}

	glGenTextures(1, &this->texture);
	glBindTexture(GL_TEXTURE_2D, this->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char* data = stbi_load(path.c_str(), &this->width, &this->height, &this->channels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
		loadedTextures[path] = this;
	}
	else {
		std::cerr << "Could not load texture " << path << std::endl;
	}
}

Par_Texture::~Par_Texture() {
	glDeleteTextures(1, &this->texture);
}

void Par_Texture::bind() const {
	if (this->texture != 0) {
		glBindTexture(GL_TEXTURE_2D, this->texture);
	}
}

int Par_Texture::getWidth() const {
	return this->width;
}

int Par_Texture::getHeight() const {
	return this->height;
}

int Par_Texture::getChannels() const {
	return this->channels;
}
