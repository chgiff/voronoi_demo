#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <string>
#include <vector>
#include <set>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tiny_obj_loader/tiny_obj_loader.h>

using namespace glm;

typedef float (*AnimationFunction)(float distance);

class Program;

struct VoronoiContainer
{
	glm::vec3 position; //seed point
	std::vector<unsigned int> faces;
    unsigned int faceOffset;

    glm::vec3 rotationAxis;
    float animationOffset;
    glm::vec3 normal;
	glm::vec3 vecToMaster;

    std::set<struct VoronoiContainer *> adjacencies;
};

struct KeyFrame
{
	float rotation;
	float time;
};

struct RotateAnimation
{
	glm::mat4 getTransform(float time, glm::vec3 axis)
	{
		unsigned int i;

		if(time <= keyFrames[0].time){
			return glm::rotate(glm::mat4(1.0), keyFrames[0].rotation, axis);
		}

		for(i = 0; i < keyFrames.size()-1; i++){
			if(time > keyFrames[i].time && time < keyFrames[i+1].time){
				float timeLerp = (time - keyFrames[i].time)/(keyFrames[i+1].time - keyFrames[i].time);
				float rotation = (1 - timeLerp)*keyFrames[i].rotation + timeLerp*keyFrames[i+1].rotation;
				return glm::rotate(glm::mat4(1.0), rotation, axis);
			}
		}

		return glm::rotate(glm::mat4(1.0), keyFrames[keyFrames.size()-1].rotation, axis);
	}

	float getTotalTime()
	{
		return keyFrames.back().time;
	}

	void addKeyFrame(float time, float rotation)
	{
		struct KeyFrame newFrame;
		newFrame.time = time;
		newFrame.rotation = rotation;
		for(unsigned int i = 0; i < keyFrames.size(); i++){
			if(time < keyFrames[i].time){
				keyFrames.insert(keyFrames.begin()+i, newFrame);
				return;
			}
		}

		keyFrames.push_back(newFrame);
	}

	private:
	std::vector<struct KeyFrame> keyFrames;
};


class Shape
{
public:
	Shape();
	virtual ~Shape();
	void createShape(tinyobj::shape_t & shape);
	void createFlat();
	void init();
	void measure();
	void resize(float scaleX, float shiftX, float scaleY, float shiftY, float scaleZ, float shiftZ);
	void draw(const std::shared_ptr<Program> prog) const;
	void generateVoronoi(std::vector<glm::vec3> seeds);
	void setAnimationFunction(AnimationFunction func);
	glm::vec3 min;
	glm::vec3 max;
	
protected:
	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	unsigned eleBufID;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
	unsigned vaoID;
	void generateNormals();

	vec3 matAmb;
	vec3 matDiff;
	vec3 matSpec;
	float shine;

	std::vector<struct VoronoiContainer> voronoiPieces;
	void drawVoronoi(const std::shared_ptr<Program> prog) const;
	bool usingVoronoi = false;
	std::vector<struct VoronoiContainer *> vertexToContainer;
	void createVoronoiContainers(std::vector<glm::vec3> seeds);
	void createRotateAnimation();
	bool isAlmostInContainer(int vertInd, struct VoronoiContainer *testContainer);
	void checkFace(int v1, int v2, int v3);
	void createPointsBetween(struct VoronoiContainer *c1, struct VoronoiContainer *c2, int v1, int v2_1, int v2_2);
	std::shared_ptr<struct RotateAnimation> rotateAnim;
	AnimationFunction animOffsetFunction;
};

#endif
