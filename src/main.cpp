#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Terrain.h"
#include "Texture.h"
#include "WindowManager.h"
#include "GLTextureWriter.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader/tiny_obj_loader.h"

#include "stb_image.h"

using namespace std;
using namespace glm;

//some functions that create various animation effects

/* animation that radiates from corner and slows down as it gets further away*/
float outSlowDownAnimation(float distance)
{
	return -2 * distance*distance;
}
/*animation that radiates in toward corner and speeds up as it gets closer*/
float inSpeedUpAnimation(float distance)
{
	return 5 * distance*distance;
}
/*animation that radiates from corner and speeds up as it gets further away*/
float outSpeedUpAnimation(float distance)
{
	return -2 * sqrt(sqrt(distance));
}


//main application class
class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;

	//Terrain
	shared_ptr<Terrain> terrain;
	shared_ptr<Texture> terrainTex;
	const vec3 terrainScale = vec3(100, 30, 100);
	const vec3 terrainShift = vec3(0, -15, 0);

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	int drawMode = 0;
	int fogMode = 0;

	vec3 curpos = vec3(0);
	vec3 lookDir = vec3(1);
	bool mouseDown = false;
	bool mouseDisabled = true;

	double mouseOffsetX = 0, mouseOffsetY = 0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		//debug, show model outline
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
 			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			drawMode = 1;
 		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
 			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			drawMode = 0;
 		}

		//switch mouse mode
		if (key == GLFW_KEY_M && action == GLFW_PRESS) {
			if(!mouseDisabled){
				glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				mouseDisabled = true;
			}
			else{
				glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				mouseDisabled = false;
			}
 		}


		//walk forward
		if(key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			curpos += normalize(lookDir);
		}
		//walk backward
		if(key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			curpos -= normalize(lookDir);
		}
		//go left
		if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			curpos -= cross(normalize(lookDir), vec3(0, 1, 0));
		}
		//go right
		if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			curpos += cross(normalize(lookDir), vec3(0, 1, 0));
		}

		//enable/disable fog effect
		else if (key == GLFW_KEY_F && action == GLFW_PRESS)
		{
			fogMode = !fogMode;
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{

	}

	 void cursorCallback(GLFWwindow *window, double posX, double posY)
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);

		mouseOffsetX = posX;
		mouseOffsetY = posY;
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0, 0, 0, 1.0);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(
			resourceDirectory + "/simple_vert.glsl",
			resourceDirectory + "/simple_frag.glsl");
		if (! prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("M");
		prog->addUniform("V");
		prog->addUniform("S");
		prog->addUniform("drawMode");
		prog->addUniform("fogMode");
		prog->addUniform("eyePos");
		//material
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");
		//point light
		prog->addUniform("pointLightPos");
		prog->addUniform("pointLightColor");
		//directional light
		prog->addUniform("dirLightVec");
		prog->addUniform("dirLightColor");

		//vertex attributes
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");


		prog->addUniform("terrainTex");
	 }

	void initGeom(const std::string& resourceDirectory)
	{
		//Initialize the geometry to render the height map terrain
		initTerrain(resourceDirectory);
	}

	/* Initializes terrain*/
	void initTerrain(const std::string& resourceDirectory)
	{
		//load in heighmap image
		terrain = make_shared<Terrain>();
		// terrain->loadImage(resourceDirectory + "/flat_flordia_heightmap.png");
		terrain->loadImage(resourceDirectory + "/home_heightmap.png");

		//create all the seeds for the voronoi containers
		std::vector<glm::vec3> voronoiSeeds;
		int numAcross = 20;
		int numPerArea = 5;
		for(int x = 0; x < numAcross; x++){
			for(int z = 0; z < numAcross; z++){
				for(int i = 0; i < numPerArea; i++){
					float wiggleX = rand() / float(RAND_MAX);
					float wiggleZ = rand() / float(RAND_MAX);
					float xPos = -1 + x*(2.0/numAcross) + (wiggleX * 2.0/numAcross);
					float zPos = -1 + z*(2.0/numAcross) + (wiggleZ * 2.0/numAcross);
					voronoiSeeds.push_back(vec3(xPos, terrain->getHeight(xPos, zPos), zPos));
				}
			}
		}

		//generate all voronoi cells and enable the animation
		terrain->setAnimationFunction(&outSpeedUpAnimation);
		terrain->generateVoronoi(voronoiSeeds);

		//initialize openGL buffers
		terrain->init();

		//initialize the texture
		terrainTex = make_shared<Texture>();
		terrainTex->initDataFromFile(resourceDirectory + "/graphiti_texture.jpg");
		terrainTex->initBuffers();
		terrainTex->setUnit(1);
		terrainTex->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	void setupLights()
	{
		glUniform3f(prog->getUniform("pointLightPos"), 0, 0, 0);
		glUniform3f(prog->getUniform("pointLightColor"), 0.3, 0.3, 0.3);

		glUniform3f(prog->getUniform("dirLightVec"), -0.56, 0.5, 0.66);
		glUniform3f(prog->getUniform("dirLightColor"), 0.7, 0.7, 0.7);
	}

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Leave this code to just draw the meshes alone */
		float aspect = width/(float)height;

		// Create the matrix stacks
		auto P = make_shared<MatrixStack>();
		auto M = make_shared<MatrixStack>();
		auto V = make_shared<MatrixStack>();
		// Apply perspective projection.
		P->pushMatrix();
		P->perspective(45.0f, aspect, 0.01f, 2000.0f);

		//calculate look direction
		double cPhi = M_PI/4 - (M_PI / (double)height)/2 * mouseOffsetY;
		double cTheta = (M_PI / (double)width) * mouseOffsetX;
		lookDir = vec3(cos(cTheta)*cos(cPhi), sin(cPhi), cos(cPhi)*cos(M_PI/2-cTheta));
		V->lookAt(curpos, curpos + lookDir, vec3(0,1,0));

		//draw all the meshes
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE,value_ptr(V->topMatrix()) );
		glUniform3f(prog->getUniform("eyePos"), curpos.x, curpos.y, curpos.z);
		glUniform1i(prog->getUniform("drawMode"), drawMode);
		glUniform1i(prog->getUniform("fogMode"), fogMode);
		setupLights();

		M->pushMatrix();
			M->loadIdentity();

			//draw terrain
			M->pushMatrix();
			M->translate(terrainShift);
			M->scale(terrainScale);
			terrainTex->bind(prog->getUniform("terrainTex"));
			int d = drawMode;
			drawMode = 2;
			glUniform1i(prog->getUniform("drawMode"), drawMode);
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE,value_ptr(M->topMatrix()) );
			terrain->draw(prog);
			drawMode = d;
			glUniform1i(prog->getUniform("drawMode"), drawMode);
			M->popMatrix();

		M->popMatrix();

		prog->unbind();

		
		P->popMatrix();
	}
};

int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
			resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(512, 512);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
			// Render scene.
			application->render();

			// Swap front and back buffers.
			glfwSwapBuffers(windowManager->getHandle());
			// Poll for and process events.
			glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
