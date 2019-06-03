#include "Texture.h"
#include "GLSL.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

Texture::Texture() :
	filename(""),
	tid(0)
{
	
}

Texture::~Texture()
{
	
}

void Texture::initDataFromColor(unsigned char r, unsigned char g, unsigned char b)
{
	width = 1;
	height = 1;
	data = (unsigned char *)stbi__malloc(3);
	data[0] = r;
	data[1] = g;
	data[2] = b;
}

void Texture::initDataFromFile(const std::string &filename)
{
	// Load texture
	int w, h, ncomps;
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load(filename.c_str(), &w, &h, &ncomps, 0);
	if(!data) {
		cerr << filename << " not found" << endl;
	}
	if(ncomps != 3) {
		cerr << filename << " must have 3 components (RGB)" << endl;
	}
	if((w & (w - 1)) != 0 || (h & (h - 1)) != 0) {
		cerr << filename << " must be a power of 2" << endl;
	}
	width = w;
	height = h;
}

void Texture::addConstantColor(unsigned char r, unsigned char g, unsigned char b)
{
	for(int w = 0; w < width; w++){
		for(int h = 0; h < height; h++){
			data[h*width + w] = ((255 - data[h*width + w] < r) ?  255: data[h*width + w] + r);
			data[h*width + w + 1] = ((255 - data[h*width + w + 1] < g) ? 255 : data[h*width + w + 1] + g);
			data[h*width + w + 2] = ((255 - data[h*width + w + 2] < b) ? 255 : data[h*width + w + 2] + b);
		}
	}
}

void Texture::initBuffers()
{
	// Generate a texture buffer object
	glGenTextures(1, &tid);
	// Bind the current texture to be the newly generated texture object
	glBindTexture(GL_TEXTURE_2D, tid);
	// Load the actual texture data
	// Base level is 0, number of channels is 3, and border is 0.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	// Generate image pyramid
	glGenerateMipmap(GL_TEXTURE_2D);
	// Set texture wrap modes for the S and T directions
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Set filtering mode for magnification and minimification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Unbind
	glBindTexture(GL_TEXTURE_2D, 0);
	// Free image, since the data is now on the GPU
	stbi_image_free(data);
	data = NULL;
}

void Texture::setWrapModes(GLint wrapS, GLint wrapT)
{
	// Must be called after init()
	glBindTexture(GL_TEXTURE_2D, tid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
}

void Texture::bind(GLint handle)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, tid);
	glUniform1i(handle, unit);
}

void Texture::unbind()
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, 0);
}
