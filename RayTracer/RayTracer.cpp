//****************************************************************************************************
// Raytracer 
// ---------------
//  o A simple ratracer using compute shader
//  o Usage: RayTracer - depth d - width w - height h
//  o		 Depth d is the actual recursion depth of the ray
//  o		 Width w and height h are the dimensions in pixel of the rendering window
//
//****************************************************************************************************

#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "ShaderClass.h"
#include "RaytraceShader.h"
#include "DrawingShaders.h"
#include "Utils.h"



//*** Scene description parameters *******************************************************************


// Camera description (looking upward the +z axis).
double eye[3] = { 750.0, 200.0, 250.0 };
double focus[3] = { 0.0, 50.0, 0.0 };
double hfov = 3.141592 / 5.0;
double dnear = 0.1;
double dfar = 10000.0;

// List of scene boxes:
int nb_boxes = 6;
double box_min[6][3] = { { -350.0, -350.0, -10.0 },{ -60.0, -60.0, 30.0 }, { -300.0, -330.0, 0.0 },  
						 { -330.0, -400.0, 0.0 },  { -300.0, 310.0, 0.0 }, { 100.0, 100.0, 40.0 } };
double box_max[6][3] = { { 350.0, 350.0, 10.0 }, { 60.0, 60.0, 150.0 },   { 330.0, -310.0, 300.0 }, 
						 { -310.0, 400.0, 300.0 }, { 330.0, 330.0, 300.0 },{ 180.0, 180.0, 120.0 } };

// Material attributes of boxes:
double box_color[6][4]    = { { 0.5, 0.5, 0.5, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.5, 0.5, 1.0 }, 
							{ 1.0, 1.0, 0.0, 1.0 }, { 0.5, 0.0, 1.0, 1.0 } , { 0.0, 0.0, 1.0, 1.0 } };

// List of scene spheres:
int nb_spheres = 2;
double sphere_center[4][3] = {  { -180.0, 180.0, 100.0 }, { 210.0, -25.0, 65.0 } };
double sphere_radius[4] = { 75.0, 55.0 };

// Material attributes of spheres:
double sphere_color[4][4]    = { { 0.0, 0.8, 0.8, 1.0 }, { 1.0, 0.0, 0.0, 1.0 } };

double obj_emmissive[4] = { 0.1, 0.1, 0.1, 1.0 };
double obj_reflection[4] = { 0.3, 0.3, 0.3, 1.0 };
// List of scene lights:
int nb_lights = 3;
double light_pos[3][3] = { { 50.0, -500.0, 800.0 }, { -350.0, 250.0, 600.0 },{ 50.0, 500.0, 800.0 } };
double light_color[3][4] = { { 0.5, 0.5, 0.5, 1.0 }, { 0.5, 0.5, 0.5, 1.0 } , { 0.5, 0.5, 0.5, 1.0 } };

//OpenGL variables
GLFWwindow  *glContext;
unsigned int quadVAO, quadVBO;
GLuint texture;
GLint 	groupSizeX, groupSizeY;
Shader _rayTracingShader, _simpleDraw;
glm::mat4 model, view , projection;


//*** Setting  The Scene     *************************************************************************

void setSceneObjects() {

	_rayTracingShader.setInt("objectsNbr", nb_boxes + nb_spheres);
	_rayTracingShader.setInt("lightsNbr", nb_lights);

	for (int i = 0; i < nb_spheres; i++)
	{
		_rayTracingShader.setFloat("vObjects[" + std::to_string(i) + "].type", 0.0);
		_rayTracingShader.setVec3("vObjects[" + std::to_string(i) + "].pos", glm::vec3(sphere_center[i][0], sphere_center[i][1], sphere_center[i][2]));
		_rayTracingShader.setFloat("vObjects[" + std::to_string(i) + "].r", sphere_radius[i]);
		_rayTracingShader.setVec4("vObjects[" + std::to_string(i) + "].color", glm::vec4(sphere_color[i][0], sphere_color[i][1], sphere_color[i][2], sphere_color[i][3]));

	}
	for (int i = 0; i < nb_boxes; i++)
	{
		_rayTracingShader.setFloat("vObjects[" + std::to_string(nb_spheres + i) + "].type", 1.0);
		_rayTracingShader.setVec3("vObjects[" + std::to_string(nb_spheres + i) + "].min", glm::vec3(box_min[i][0], box_min[i][1], box_min[i][2]));
		_rayTracingShader.setVec3("vObjects[" + std::to_string(nb_spheres + i) + "].max", glm::vec3(box_max[i][0], box_max[i][1], box_max[i][2]));
		_rayTracingShader.setVec4("vObjects[" + std::to_string(nb_spheres + i) + "].color", glm::vec4(box_color[i][0], box_color[i][1], box_color[i][2], box_color[i][3]));
	}
	for (int i = 0; i < nb_lights; i++)
	{
		_rayTracingShader.setVec3("vLights[" + std::to_string(i) + "].pos", glm::vec3(light_pos[i][0], light_pos[i][1], light_pos[i][2]));
		_rayTracingShader.setVec4("vLights[" + std::to_string(i) + "].color", glm::vec4(light_color[i][0], light_color[i][1], light_color[i][2], light_color[i][3]));
	}

	_rayTracingShader.setVec4("emission", glm::vec4(obj_emmissive[0], obj_emmissive[1], obj_emmissive[2], obj_emmissive[3]));
	_rayTracingShader.setVec4("reflection", glm::vec4(obj_reflection[0], obj_reflection[1], obj_reflection[2], obj_reflection[3]));

}
bool setGLVariables(const int width,const int height)
{

	view = glm::lookAt(glm::vec3(eye[0], eye[1], eye[2]), glm::vec3(focus[0], focus[1], focus[2]), glm::vec3(0.0f, 0.0f, 1.0f));
	projection = glm::perspective((GLfloat)hfov, (GLfloat)width/ (GLfloat)height, (GLfloat)dnear, (GLfloat)dfar);

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		error_callback(1, "GLFW initialisation failed!");
		glfwTerminate();
		return false;
	}
	// Setup GLFW window properties
	// OpenGL version
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);


	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

	// Create the window
	glContext = glfwCreateWindow(width, height, "Rendering", NULL, NULL);

	if (!glContext)
	{
		error_callback(1, "GLFW window creation failed!");
		glfwTerminate();
		return false;
	}
	// Set context for GLEW to use
	glfwMakeContextCurrent(glContext);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		error_callback(1, "Failed to initialize GLAD");
		return false;
	}


	//Initializing the compute shader
	if (!_rayTracingShader.initComputeShader(rayTraceCS))
	{
		error_callback(1, "Raytracing Shader Error\n");
		return false;
	}


	glfwSwapInterval(1);
	glfwShowWindow(glContext);

	//Setting the texture for the compute shader
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Preparing the compute Shader
	_rayTracingShader.use();
	setSceneObjects();
	int sizes[3];
	glGetProgramiv(_rayTracingShader.getID(), GL_COMPUTE_WORK_GROUP_SIZE, sizes);
	// we only need X and Y groups
	groupSizeX = sizes[0];
	groupSizeY = sizes[1];

	glUseProgram(0);


	//Initializing the shaders for display
	if (!_simpleDraw.init(rayTraceVS, rayTraceFS))
	{
		fprintf(stdout, "Simple Draw ShaderError\n");
		return false;
	}
	init_Quad(_simpleDraw.getID(), quadVAO, quadVBO);

	//Setting texture coordinates to the shader
	glUseProgram(_simpleDraw.getID());
	int texUniform = glGetUniformLocation(_simpleDraw.getID(), "tex");
	glUniform1i(texUniform, 0);
	glUseProgram(0);

	return true;
}

//*** Rendering ***********************************************************************************

void render(int width , int height, int depth)
{
	//Clearing the rendering 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height);
	glFlush();
	_rayTracingShader.use();

	// Set shader uniform input
	_rayTracingShader.setVec3("eye", glm::vec3(eye[0], eye[1], eye[2]));
	_rayTracingShader.setMat4("inversinvProjectionView", glm::inverse(projection * view));
	_rayTracingShader.setInt("depthMax", depth);
	_rayTracingShader.setFloat("dnear", (GLfloat)dnear);
	_rayTracingShader.setFloat("dfar", (GLfloat)dfar);

	// Bind level 0 of framebuffer texture as writable image in the shader
	glBindImageTexture(0, texture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// Compute appropriate invocation dimension (closest next power of 2)
	int worksizeX = pow(2, ceil(log((float)width) / log(2))); 
	int worksizeY = pow(2, ceil(log((float)height) / log(2)));

	// Invoke the compute shader
	glDispatchCompute(worksizeX / groupSizeX, worksizeY / groupSizeY, 1);

	// Reset image binding
	glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(0);

	// Draw the rendered image on the screen using textured full-scree  quad
	_simpleDraw.use();
	glBindVertexArray(quadVAO);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

//*** main *******************************************************************************************

int main( int argc, char** argv )
{
  // Retrieving input parameters:

  int i, depth, width, height;
  
  if( argc < 7 )
  {
	  error_callback(1, "Usage: RayTracer -depth d -width w -height h.\n"\
                "Depth'd' is the actual recursion depth of the ray-tracer.\n"\
                "Width 'w' and height 'h' are the dimensions in pixel of the rendering window.\n" );
  }

  // Check inputs
  for( i = 1; i < ( argc - 1 ); i++ )
  {
    if( strcmp( argv[ i ], "-depth" ) == 0 )
    {
      sscanf( argv[ i + 1 ], "%d", &depth );
    }
    if( strcmp( argv[ i ], "-width" ) == 0 )
    {
      sscanf( argv[ i + 1 ], "%d", &width );
    }
    if( strcmp( argv[ i ], "-height" ) == 0 )
    {
      sscanf( argv[ i + 1 ], "%d", &height );
    }
  }

  if( width <= 0 || height <= 0 )
  {
	  error_callback(1, "RayTracer: Error, invalid image dimensions.\n" );
  }

  depth = ( depth < 0 ) ? 0 : ( depth > 100 ) ? 100 : depth;
  
  //Preparing OpenGL environment
  if (!setGLVariables(width, height))
  {
	  error_callback(1, "Could not init!\n");
	  return -1;
  }

  //Rendering
  glfwSetInputMode(glContext, GLFW_STICKY_KEYS, GL_TRUE);

  while (glfwGetKey(glContext, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(glContext) == 0)
  {
	  glfwPollEvents();
	  render(width, height, depth);
	  glfwSwapBuffers(glContext);
  }


  //Clean up
  glfwTerminate();

  return 1;
}

