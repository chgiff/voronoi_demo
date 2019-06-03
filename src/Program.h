
#pragma  once

#ifndef LAB471_PROGRAM_H_INCLUDED
#define LAB471_PROGRAM_H_INCLUDED

#include <map>
#include <string>

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include "emscripten.h"
#else
#include <glad/glad.h>
#endif


std::string readFileAsString(const std::string &fileName);

class Program
{

public:

	void setVerbose(const bool v) { verbose = v; }
	bool isVerbose() const { return verbose; }

	void setShaderNames(const std::string &v, const std::string &f);
	virtual bool init();
	virtual void bind();
	virtual void unbind();

	void addAttribute(const std::string &name);
	void addUniform(const std::string &name);
	GLint getAttribute(const std::string &name) const;
	GLint getUniform(const std::string &name) const;

	GLuint pid;

protected:

	std::string vShaderName;
	std::string fShaderName;

private:


	std::map<std::string, GLint> attributes;
	std::map<std::string, GLint> uniforms;
	bool verbose;

};

#endif // LAB471_PROGRAM_H_INCLUDED
