#ifndef TEXTURE_Pariticle_H
#define TEXTURE_Pariticle_H

#include <map>
#include <string>

class Par_Texture {
public:
	Par_Texture();
	Par_Texture(const std::string& path);
	~Par_Texture();

	Par_Texture(const Par_Texture&) = delete;
	Par_Texture& operator=(const Par_Texture&) = delete;

	int getWidth() const;
	int getHeight() const;
	int getChannels() const;
	void bind() const;
	void loadFromFile(const std::string& path);

private:
	int width;
	int height;
	int channels;
	unsigned int texture;

	static std::map<std::string, Par_Texture*> loadedTextures;
};

#endif