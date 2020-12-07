#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "Utils.h"

void error_callback(int err_code, const char* err_str)
{
	fprintf(stderr, "GLFW Error: %s\n", err_str);
	exit(err_code);
}

bool check_FB_Status()
{
	GLenum e = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	bool FB_status = false;
	switch (e) {
	case GL_FRAMEBUFFER_COMPLETE:
	{
		fprintf(stdout, "FBO OK");
		FB_status = true;
	}
	break;
	case GL_FRAMEBUFFER_UNDEFINED:
		fprintf(stdout, "FBO Undefined");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		fprintf(stdout, "FBO Incomplete Attachment");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		fprintf(stdout, "FBO Missing Attachment");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		fprintf(stdout, "FBO Incomplete Draw Buffer");
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		fprintf(stdout, "FBO Unsupported");
		break;
	default:
		fprintf(stdout, "FBO Problem?");
	}
	return FB_status;
}

void init_Quad(unsigned int  shaderID, unsigned int  & quadVAO, unsigned int  & quadVBO)
{
	float quadVertices[] = {
		//Position	   //Coordinates
		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f,  1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f
	};

	// setup plane VAO
	if (quadVBO)
	{
		glDeleteBuffers(1, &quadVBO);
		quadVBO = 0;
	}

	if (quadVAO)
	{
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVAO);
		quadVAO = 0;
	}
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);


	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	GLuint pos_attrib = glGetAttribLocation(shaderID, "aPos");
	glEnableVertexAttribArray(pos_attrib);
	glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	GLuint tex_attrib = glGetAttribLocation(shaderID, "aTexCoords");
	glEnableVertexAttribArray(tex_attrib);
	glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}