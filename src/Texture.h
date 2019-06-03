#pragma once
#ifndef __Texture__
#define __Texture__

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include "emscripten.h"
#else
#include <glad/glad.h>
#endif

#include <string>

class Texture
{
public:
	Texture();
	virtual ~Texture();
	void initDataFromFile(const std::string &filename);
	void initDataFromColor(unsigned char r, unsigned char g, unsigned char b);
	void addConstantColor(unsigned char r, unsigned char g, unsigned char b);
	void initBuffers();
	void setUnit(GLint u) { unit = u; }
	GLint getUnit() const { return unit; }
	void bind(GLint handle);
	void unbind();
	void setWrapModes(GLint wrapS, GLint wrapT); // Must be called after init()
	GLint getID() const { return tid;}
private:
	std::string filename;
	int width;
	int height;
	unsigned char *data;
	GLuint tid;
	GLint unit;
	
};

#endif
