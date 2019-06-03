#include "Shape.h"
#include <iostream>
#include <assert.h>
#include "MatrixStack.h"

#include "GLSL.h"
#include "Program.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

using namespace std;
using namespace glm;

float defaultAnim(float distance)
{
	return 5*distance;
}

Shape::Shape() :
	eleBufID(0),
	posBufID(0),
	norBufID(0),
	texBufID(0), 
   vaoID(0)
{
	min = glm::vec3(0);
	max = glm::vec3(0);
	animOffsetFunction = &defaultAnim;
}

Shape::~Shape()
{
}

/* copy the data from the tinyobj shape to this object */
void Shape::createShape(tinyobj::shape_t & shape)
{
		posBuf = shape.mesh.positions;
		norBuf = shape.mesh.normals;
		texBuf = shape.mesh.texcoords;
		eleBuf = shape.mesh.indices;
}

/* debug function that creates a flat quad */
void Shape::createFlat()
{
	posBuf.push_back(-1);
	posBuf.push_back(0);
	posBuf.push_back(-1);
	texBuf.push_back(0);
	texBuf.push_back(0);

	posBuf.push_back(-1);
	posBuf.push_back(0);
	posBuf.push_back(1);
	texBuf.push_back(0);
	texBuf.push_back(1);
	
	posBuf.push_back(1);
	posBuf.push_back(0);
	posBuf.push_back(1);
	texBuf.push_back(1);
	texBuf.push_back(1);
	
	posBuf.push_back(1);
	posBuf.push_back(0);
	posBuf.push_back(-1);
	texBuf.push_back(1);
	texBuf.push_back(0);

	eleBuf.push_back(0);
	eleBuf.push_back(1);
	eleBuf.push_back(2);

	eleBuf.push_back(0);
	eleBuf.push_back(2);
	eleBuf.push_back(3);
}

/* Calculate the extents of the shape object */
void Shape::measure() {
	float minX, minY, minZ;
   	float maxX, maxY, maxZ;

   	minX = minY = minZ = 1.1754E+38F;
   	maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   	for (size_t v = 0; v < posBuf.size() / 3; v++) {
		if(posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
		if(posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];

		if(posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
		if(posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];

		if(posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
		if(posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
	max.x = maxX;
	max.y = maxY;
	max.z = maxZ;
}

/* resize and move the shape*/
void Shape::resize(float scaleX, float shiftX, float scaleY, float shiftY, float scaleZ, float shiftZ)
{
	// Go through all verticies shift and scale them
	for (size_t v = 0; v < posBuf.size() / 3; v++)
	{
		posBuf[3*v+0] = (posBuf[3*v+0] - shiftX) * scaleX;
		posBuf[3*v+1] = (posBuf[3*v+1] - shiftY) * scaleY;
		posBuf[3*v+2] = (posBuf[3*v+2] - shiftZ) * scaleZ;
	}
}

/* Generate normals for shape that doesn't have them */
void Shape::generateNormals()
{
	//initialize normal buffer to all 0s
	norBuf.resize(posBuf.size());
	fill(norBuf.begin(), norBuf.end(), 0);

	//get normal vector for every face and update adjacent vertices
	for (size_t f = 0; f < eleBuf.size() / 3; f++) {
		//take cross product to get normal for the face
		vec3 vert1 = vec3(posBuf[3*eleBuf[3*f]], posBuf[3*eleBuf[3*f] + 1], posBuf[3*eleBuf[3*f] + 2]);
		vec3 vert2 = vec3(posBuf[3*eleBuf[3*f+1]], posBuf[3*eleBuf[3*f+1] + 1], posBuf[3*eleBuf[3*f+1] + 2]);
		vec3 vert3 = vec3(posBuf[3*eleBuf[3*f+2]], posBuf[3*eleBuf[3*f+2] + 1], posBuf[3*eleBuf[3*f+2] + 2]);
		

		vec3 v1 = vert2 - vert1;
		vec3 v2 = vert3 - vert1;

		vec3 normal = cross(v1, v2);

		//add normal for the face to each of it's vertices
		norBuf[3*eleBuf[3*f]] += normal.x;
		norBuf[3*eleBuf[3*f]+1] += normal.y;
		norBuf[3*eleBuf[3*f]+2] += normal.z;

		norBuf[3*eleBuf[3*f+1]] += normal.x;
		norBuf[3*eleBuf[3*f+1]+1] += normal.y;
		norBuf[3*eleBuf[3*f+1]+2] += normal.z;

		norBuf[3*eleBuf[3*f+2]] += normal.x;
		norBuf[3*eleBuf[3*f+2]+1] += normal.y;
		norBuf[3*eleBuf[3*f+2]+2] += normal.z;
	}

	//normalize all vectors
	for (size_t v = 0; v < norBuf.size() / 3; v++){
		float len = sqrt(norBuf[3*v]*norBuf[3*v] + norBuf[3*v+1]*norBuf[3*v+1] + norBuf[3*v+2]*norBuf[3*v+2]);
		norBuf[3*v] /= len;
		norBuf[3*v+1] /= len;
		norBuf[3*v+2] /= len;
	}
}

/* Initialize openGL buffers */
void Shape::init()
{
   // Initialize the vertex array object
   glGenVertexArrays(1, &vaoID);
   glBindVertexArray(vaoID);

	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	if(norBuf.empty()) {
		generateNormals();
		cout << "Generated normals" << endl;
	}
	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	
	// Send the texture array to the GPU
	if(texBuf.empty()) {
		texBufID = 0;
	} else {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the element array to the GPU
	glGenBuffers(1, &eleBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	int err = glGetError();
	assert(err == GL_NO_ERROR);
}


/* draw the shape */
void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

	// if option to generate voronoi shapes has been set, then use that draw function instead
	if(usingVoronoi){
		return drawVoronoi(prog);
	}

   glBindVertexArray(vaoID);
	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if(h_nor != -1 && norBufID != 0) {
		GLSL::enableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	if (texBufID != 0) {	
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");
		if(h_tex != -1 && texBufID != 0) {
			GLSL::enableVertexAttribArray(h_tex);
			glBindBuffer(GL_ARRAY_BUFFER, texBufID);
			glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
		}
	}
	
	// Bind element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	
	// Draw
	auto S = std::make_shared<MatrixStack>();
	S->loadIdentity();
	glUniformMatrix4fv(prog->getUniform("S"), 1, GL_FALSE, glm::value_ptr(S->topMatrix()));	
	glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);
	
	// Disable and unbind
	if(h_tex != -1) {
		GLSL::disableVertexAttribArray(h_tex);
	}
	if(h_nor != -1) {
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


/* 
* Sets up the animation keyframes for this shape 
* Does a rotation 90 deg and back over the span of 20 seconds 
*/
void Shape::createRotateAnimation()
{
	rotateAnim = make_shared<struct RotateAnimation>();
	rotateAnim->addKeyFrame(0,0);
	rotateAnim->addKeyFrame(1, M_PI/2);
	rotateAnim->addKeyFrame(10, M_PI/2);
	rotateAnim->addKeyFrame(11, 0);
	rotateAnim->addKeyFrame(20, 0);
}

/* 
* sets the function that controls how the animation behaves
*/
void Shape::setAnimationFunction(AnimationFunction func)
{
	animOffsetFunction = func;
}


/* Euclidian distance */
inline float distance(float x1, float y1, float z1, float x2, float y2, float z2)
{
	return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2);
}

//returns the percentage between the two points that the new point should be (0 closest to p1, 1.0 closest to p2)
//new point will be equal distance from the two voronoi centers
float newPointLerp(struct VoronoiContainer *center1, struct VoronoiContainer *center2, glm::vec3 & p1, glm::vec3 & p2)
{
	glm::vec3 newPt;

	//generate plane 
	glm::vec3 planeNorm = center2->position - center1->position;
	glm::vec3 planePoint = (center1->position + center2->position)/2.0f;

	//calculate intersection between plane and line between points
	float t = glm::dot(planeNorm, planePoint - p1) / glm::dot(planeNorm, p2 - p1);

	return t;
}

//find the closest voronoi container for a given x,y,z position
struct VoronoiContainer *closestContainer(std::vector<struct VoronoiContainer> & containers, float x, float y, float z)
{
	float closestDist;
	struct VoronoiContainer *closest = NULL;
	if(containers.size() == 0){
		//error
		std::cerr << "error containers empty" << std::endl;
		return NULL;
	}
	closestDist = distance(x, y, z, containers[0].position.x, containers[0].position.y, containers[0].position.z);
	closest = &containers[0];

	for(unsigned int i = 1; i < containers.size(); i++){
		float d = distance(x, y, z, containers[i].position.x, containers[i].position.y, containers[i].position.z);
		if(d < closestDist){
			closestDist = d;
			closest = &containers[i];
		}
	}

	return closest;
}

/* 
* checks if a point is right on the border of being in a voronoi container 
* i.e., checks if point's actual container and the test container are the same distance away within an epsilon
*/
bool Shape::isAlmostInContainer(int vertInd, struct VoronoiContainer *testContainer)
{
	struct VoronoiContainer *actualContainer = vertexToContainer[vertInd];
	float EPSILON = 0.0001;
	float d1 = distance(posBuf[3*vertInd], posBuf[3*vertInd+1], posBuf[3*vertInd+2], testContainer->position.x, testContainer->position.y, testContainer->position.z);
	float d2 = distance(posBuf[3*vertInd], posBuf[3*vertInd+1], posBuf[3*vertInd+2], actualContainer->position.x, actualContainer->position.y, actualContainer->position.z);

	return (abs(d1-d2) < EPSILON);
}

//genreates the points to split up a triangle that spans between two different voronoi containers
//args are two containers, then index of vertex in first container, and indexes of two points in second container
//v2_1 is clockwise from v1
void Shape::createPointsBetween(struct VoronoiContainer *c1, struct VoronoiContainer *c2, int v1, int v2_1, int v2_2)
{
	glm::vec3 p1, p2_1, p2_2;
	p1 = glm::vec3(posBuf[3*v1], posBuf[3*v1+1], posBuf[3*v1+2]);
	p2_1 = glm::vec3( posBuf[3*v2_1], posBuf[3*v2_1+1], posBuf[3*v2_1+2]);
	p2_2 = glm::vec3(posBuf[3*v2_2], posBuf[3*v2_2+1], posBuf[3*v2_2+2]);

	//create first new point
	float newP1Lerp = newPointLerp(c1, c2, p1, p2_1);
	glm::vec3 newP1 = p1*(1-newP1Lerp) + p2_1*newP1Lerp;
	posBuf.push_back(newP1.x);
	posBuf.push_back(newP1.y);
	posBuf.push_back(newP1.z);
	int newP1Index = posBuf.size()/3-1;

	//create second new point
	float newP2Lerp = newPointLerp(c1, c2, p1, p2_2);
	glm::vec3 newP2 = p1*(1-newP2Lerp) + p2_2*newP2Lerp;
	posBuf.push_back(newP2.x);
	posBuf.push_back(newP2.y);
	posBuf.push_back(newP2.z);
	int newP2Index = posBuf.size()/3-1;

	//fix texBuf if it exists
	if(texBuf.size() > 0){
		float tex1, tex2;

		//newP1 x, y
		tex1 = texBuf[2*v1];
		tex2 = texBuf[2*v2_1];
		texBuf.push_back(tex1*(1-newP1Lerp) + tex2*newP1Lerp);
		tex1 = texBuf[2*v1+1];
		tex2 = texBuf[2*v2_1+1];
		texBuf.push_back(tex1*(1-newP1Lerp) + tex2*newP1Lerp);

		//newP2 x,y
		tex1 = texBuf[2*v1];
		tex2 = texBuf[2*v2_2];
		texBuf.push_back(tex1*(1-newP2Lerp) + tex2*newP2Lerp);
		tex1 = texBuf[2*v1+1];
		tex2 = texBuf[2*v2_2+1];
		texBuf.push_back(tex1*(1-newP2Lerp) + tex2*newP2Lerp);
	}

	struct VoronoiContainer *newP1Container = closestContainer(voronoiPieces, newP1.x, newP1.y, newP1.z);
	vertexToContainer.push_back(newP1Container);
	struct VoronoiContainer *newP2Container = closestContainer(voronoiPieces, newP2.x, newP2.y, newP2.z);
	vertexToContainer.push_back(newP2Container);

	if(isAlmostInContainer(newP1Index, c1) || isAlmostInContainer(newP1Index, c2)){
		c2->adjacencies.insert(c1);
		c1->adjacencies.insert(c2);
		
		c2->faces.push_back(v2_2);
		c2->faces.push_back(newP1Index);
		c2->faces.push_back(v2_1);

		// if(isAlmostInContainer(newP2Index, c1) || isAlmostInContainer(newP2Index, c2)){
			c2->faces.push_back(newP2Index);
			c2->faces.push_back(newP1Index);
			c2->faces.push_back(v2_2);

			c1->faces.push_back(v1);
			c1->faces.push_back(newP1Index);
			c1->faces.push_back(newP2Index);
		// }
		// else{
		// 	checkFace(v1, newP1Index, newP2Index);
		// 	checkFace(newP2Index, newP1Index, v2_2);
		// }
	}
	// else if(isAlmostInContainer(newP2Index, c1) || isAlmostInContainer(newP2Index, c2))
	// {
	// 	c2->adjacencies.insert(c1);
	// 	c1->adjacencies.insert(c2);

	// 	checkFace(v1, newP1Index, newP2Index);
	// 	checkFace(newP2Index, newP1Index, v2_2);
	// 	checkFace(v2_2, newP1Index, v2_1);
	// }
	// else{
	// 	checkFace(v1, newP1Index, newP2Index);
	// 	checkFace(newP2Index, newP1Index, v2_2);
	// 	checkFace(v2_2, newP1Index, v2_1);
	// }
}

//check a face to see if it is all in same voronoi container or spans multiple and needs to be split
//args are vertex indexes for 3 vertices on face
void Shape::checkFace(int v1, int v2, int v3)
{
	struct VoronoiContainer *c1, *c2, *c3;

	c1 = vertexToContainer[v1];
	c2 = vertexToContainer[v2];
	c3 = vertexToContainer[v3];


	if(isAlmostInContainer(v2, c1) && isAlmostInContainer(v3, c1)){
		// all in one container
		c1->faces.push_back(v1);
		c1->faces.push_back(v2);
		c1->faces.push_back(v3);
	}
	else if(isAlmostInContainer(v3, c2) && isAlmostInContainer(v1, c2)){
		// all in one container
		c2->faces.push_back(v1);
		c2->faces.push_back(v2);
		c2->faces.push_back(v3);
	}
	else if(isAlmostInContainer(v1, c3) && isAlmostInContainer(v2, c3)){
		// all in one container
		c3->faces.push_back(v1);
		c3->faces.push_back(v2);
		c3->faces.push_back(v3);
	}
	//two cases where v1 and v2 are together
	else if(isAlmostInContainer(v2, c1)){
		createPointsBetween(c3, c1, v3, v1, v2);
	}
	else if(isAlmostInContainer(v1, c2)){
		createPointsBetween(c3, c2, v3, v1, v2);
	}
	//two cases where v1 and v3 are together
	else if(isAlmostInContainer(v3, c1)){
		createPointsBetween(c2, c1, v2, v3, v1);
	}
	else if(isAlmostInContainer(v1, c3)){
		createPointsBetween(c2, c3, v2, v3, v1);
	}
	//two cases where v2 and v3 are together
	else if(isAlmostInContainer(v3, c2)){
		createPointsBetween(c1, c2, v1, v2, v3);
	}
	else if(isAlmostInContainer(v2, c3)){
		createPointsBetween(c1, c3, v1, v2, v3);
	}
	//case where they are all seperate
	else{
		createPointsBetween(c1, c2, v1, v2, v3);
	}
}

//creates all the voronoi containers from a list of seed points
void Shape::createVoronoiContainers(std::vector<glm::vec3> seeds)
{
	if(seeds.size() < 1){
		std::cerr << "Must have at least one voronoi seed" << std::endl;
		return;
	}

	auto S = std::make_shared<MatrixStack>();

	for(unsigned int i = 0; i < seeds.size(); i++){
		struct VoronoiContainer newPiece;
		newPiece.position = seeds[i];
		newPiece.normal = glm::vec3(0);
		newPiece.animationOffset = animOffsetFunction(distance(seeds[i].x, seeds[i].y, seeds[i].z, seeds[0].x, seeds[0].y, seeds[0].z));

		//set the vector that points toward the master (assumed to be seed 0)
		newPiece.vecToMaster = seeds[0] - seeds[i];
		voronoiPieces.push_back(newPiece);

		//also update master so it points in the general direction of all others
		voronoiPieces[0].vecToMaster += (seeds[0] - seeds[i]);
	}
}

//public function that is called on shape to setup everything voronoi
void Shape::generateVoronoi(std::vector<glm::vec3> seeds)
{
	usingVoronoi = true;
	generateNormals();
	createVoronoiContainers(seeds);
	createRotateAnimation();

	//go through every point and determine which container it falls in
	for(unsigned int i = 0; i < posBuf.size()/3; i++){
		struct VoronoiContainer *closest = closestContainer(voronoiPieces, posBuf[3*i], posBuf[3*i+1], posBuf[3*i+2]);
		closest->normal += glm::vec3(norBuf[3*i], norBuf[3*i+1], norBuf[3*i+2]);
		vertexToContainer.push_back(closest);
	}

	//go through every face and split the faces so all points are in the same conatiner
	for(unsigned int i = 0; i < eleBuf.size()/3; i++){
		int v1, v2, v3;
		v1 = eleBuf[3*i];
		v2 = eleBuf[3*i+1];
		v3 = eleBuf[3*i+2];
		checkFace(v1, v2, v3);
	}

	//update element buffer with all the new faces
	eleBuf.clear();
	for(unsigned int i = 0; i < voronoiPieces.size(); i++){
		voronoiPieces[i].faceOffset = eleBuf.size();
		eleBuf.insert(eleBuf.end(), voronoiPieces[i].faces.begin(), voronoiPieces[i].faces.end());
		voronoiPieces[i].normal = glm::normalize(voronoiPieces[i].normal);
	}

	//set the rotation axis for each voronoi piece
	for(struct VoronoiContainer & container : voronoiPieces){
		container.rotationAxis = glm::cross(container.vecToMaster, container.normal);
	}
}

//special case of draw for voronoi pieces
//assumes GLSL shader has an S transform matrix that should be multiplied before MVP matricies
// i.e. P*V*M*S*vertPos
void Shape::drawVoronoi(const std::shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

    glBindVertexArray(vaoID);
	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if(h_nor != -1 && norBufID != 0) {
		GLSL::enableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	if (texBufID != 0) {	
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");
		if(h_tex != -1 && texBufID != 0) {
			GLSL::enableVertexAttribArray(h_tex);
			glBindBuffer(GL_ARRAY_BUFFER, texBufID);
			glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
		}
	}
	
	// Bind element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	
	// Draw
	auto S = std::make_shared<MatrixStack>();
	float totTime = rotateAnim->getTotalTime();
	for(struct VoronoiContainer piece : voronoiPieces){
		S->loadIdentity();
		//move to origin, rotate, move back
		S->translate(1.0f*piece.position);
		S->multMatrix(rotateAnim->getTransform(fmod(glfwGetTime() + piece.animationOffset, totTime), piece.rotationAxis));
		S->translate(-1.0f*piece.position);
		
		glUniformMatrix4fv(prog->getUniform("S"), 1, GL_FALSE, glm::value_ptr(S->topMatrix()));
		glDrawElements(GL_TRIANGLES, (int)piece.faces.size(), GL_UNSIGNED_INT, (void *)(sizeof(unsigned int) * piece.faceOffset));
	}

	// Disable and unbind
	if(h_tex != -1) {
		GLSL::disableVertexAttribArray(h_tex);
	}
	if(h_nor != -1) {
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}