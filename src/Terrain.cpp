#include "Terrain.h"
#include "MatrixStack.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "GLSL.h"
#include "Program.h"

#include "stb_image.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>

int imgWidth, imgHeight;
float *heights;


void Terrain::loadImage(const std::string &heightMap)
{
    // Load heightmap image
	int w, h, ncomps;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(heightMap.c_str(), &w, &h, &ncomps, 0);
	if(! data)
	{
		std::cerr << heightMap << " not found" << std::endl;
	}
	imgWidth = w;
	imgHeight = h;
	heights = (float*)malloc(sizeof(float) * imgWidth * imgHeight);

	for(int i = 0; i < w*h; i ++){
		//get height as value between 0 and 1
		float curHeight;
		//convert rgb to greyscale
		if(ncomps == 3){
			curHeight = (data[i*3]*0.3 + data[i*3+1]*0.59 + data[i*3+2]*0.11)/255.0;
		}
		//convert rgba to greyscale
		if(ncomps == 4){
			curHeight = (data[i*4]*0.3 + data[i*4+1]*0.59 + data[i*4+2]*0.11)/255.0;
		}
		//just use greyscale
		else if(ncomps == 1){
			curHeight = data[i]/255.0;
		}
		//???
		else{
			std::cerr << "Weird number of color components" << std::endl;
			break;
		}
		
		posBuf.push_back(-1 + 2*(i%w)/(float)w); //x
		posBuf.push_back(curHeight); //y
		posBuf.push_back(-1 + 2*(i/h)/(float)h); //z

		heights[i] = curHeight;

		texBuf.push_back((i%w)/(float)w);
		texBuf.push_back((i/h)/(float)h);
	}	


	//setup indexed face set
	for(int x = 0; x < w-1; x++){
		for(int y = 0; y < h-1; y++){
			/*
			* |\
			* |_\
			*/
			//vertices go counterclockwise
			eleBuf.push_back(y*w + x);
			eleBuf.push_back((y+1)*w + x);
			eleBuf.push_back((y+1)*w + x+1);

			/*
			* ___
			* \ |
			*  \|
			*/
			//vertices go counterclockwise
			eleBuf.push_back((y+1)*w + x+1);
			eleBuf.push_back(y*w + x+1);
			eleBuf.push_back(y*w + x);
		}
	}

	stbi_image_free(data);

	generateNormals();
}

//get's the height at a given location (between -1 an 1)
float Terrain::getHeight(float xpos, float ypos)
{
	float x = (1+xpos)/2.0 * imgWidth;
	int x1 = floor(x);
	if(x1 > imgWidth - 1) x1 = imgWidth - 1;
	if(x1 < 0) x1 = 0;
	int x2 = ceil(x);
	if(x2 > imgWidth - 1) x2 = imgWidth - 1;
	if(x2 < 0) x2 = 0;
	float percentX = (x-x1);

	float y = (1+ypos)/2.0 * imgHeight;
	int y1 = floor(y);
	if(y1 > imgHeight - 1) y1 = imgHeight - 1;
	if(y1 < 0) y1 = 0;
	int y2 = ceil(y);
	if(y2 > imgHeight - 1) y2 = imgHeight - 1;
	if(y2 < 0) y2 = 0;
	float percentY = (y-y1);

	float xh1 = (heights[y1*imgWidth + x1] * (1-percentX)) + (heights[y1*imgWidth + x2] * percentX);
	float xh2 = (heights[y2*imgWidth + x1] * (1-percentX)) + (heights[y2*imgWidth + x2] * percentX);

	return (xh1 * (1-percentY)) + (xh2 * percentY);
}

//get the rotation matrix of current face
glm::vec2 Terrain::getRotation(float xpos, float ypos, glm::vec3 terrainScale)
{
	float x = (1+xpos)/2.0 * imgWidth;
	int x1 = floor(x);
	if(x1 > imgWidth - 1) x1 = imgWidth - 1;
	if(x1 < 0) x1 = 0;
	int x2 = ceil(x);
	if(x2 > imgWidth - 1) x2 = imgWidth - 1;
	if(x2 < 0) x2 = 0;
	float percentX = (x-x1);

	float y = (1+ypos)/2.0 * imgHeight;
	int y1 = floor(y);
	if(y1 > imgHeight - 1) y1 = imgHeight - 1;
	if(y1 < 0) y1 = 0;
	int y2 = ceil(y);
	if(y2 > imgHeight - 1) y2 = imgHeight - 1;
	if(y2 < 0) y2 = 0;
	float percentY = (y-y1);

	glm::vec3 normal = glm::vec3(0);

	int index = y1*imgWidth + x1;
	normal += (glm::vec3(norBuf[3*index], norBuf[3*index+1], norBuf[3*index+2]) * (1-percentX) * (1-percentY));
	index = y1*imgWidth + x2;
	normal += (glm::vec3(norBuf[3*index], norBuf[3*index+1], norBuf[3*index+2]) * (percentX) * (1-percentY));
	index = y2*imgWidth + x1;
	normal += (glm::vec3(norBuf[3*index], norBuf[3*index+1], norBuf[3*index+2]) * (1-percentX) * (percentY));
	index = y2*imgWidth + x2;
	normal += (glm::vec3(norBuf[3*index], norBuf[3*index+1], norBuf[3*index+2]) * (percentX) * (percentY));

	normal = glm::vec3(normalTransform * glm::vec4(normal.x, normal.y, normal.z, 0));
	normal = glm::normalize(normal);

	float xRot = atan(normal.x/normal.y);
	float zRot = atan(normal.z/normal.y);

	return glm::vec2(xRot, zRot);
}

void Terrain::setTerrainScale(float x, float y, float z)
{
	terrainScaleVec = glm::vec3(x, y, z);
	normalTransform = glm::transpose(glm::inverse(glm::scale(glm::mat4(1.0f), terrainScaleVec)));
}
