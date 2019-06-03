#pragma once
#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include <string>
#include <vector>
#include <set>
#include <memory>
#include "Shape.h"

#include <glm/gtc/type_ptr.hpp>

class Program;

class Terrain: public Shape
{
    public: 
        void loadImage(const std::string &heightMap);
        // void generateVoronoi();
        // void init();
        // void draw(const std::shared_ptr<Program> prog) const;
        
        //get's the height at a given location (between 0 an 1)
        float getHeight(float xpos, float ypox);
        glm::vec2 getRotation(float xpos, float ypox, glm::vec3 terrainScale);

        void setTerrainScale(float x, float y, float z);
        glm::vec3 getTerrainScale(){return terrainScaleVec;}

    private:
        glm::vec3 terrainScaleVec;
        glm::mat4 normalTransform;

        // std::vector<unsigned int> eleBuf;
        // std::vector<float> posBuf;
        // std::vector<float> norBuf;
        // std::vector<float> texBuf;
        // unsigned eleBufID;
        // unsigned posBufID;
        // unsigned norBufID;
        // unsigned texBufID;
        // unsigned vaoID;
        // void generateNormals();


	    // std::vector<struct VoronoiContainer> voronoiPieces;
        // void drawVoronoi(const std::shared_ptr<Program> prog) const;
        // bool usingVoronoi = false;
};

#endif