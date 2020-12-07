#include "ShaderClass.h"

#include <stdio.h>
#include <iostream>
using namespace std;

bool Shader::init(const char* vertexShader, const char* fragmentShader )
{

	if (vertexShader == nullptr)
	{
		fprintf(stdout, "Null Vertex Shader Code\n");
		return false;
	}
	// shader Program
	_ID = glCreateProgram();

	const char* vShaderCode = vertexShader;

	// vertex shader
	unsigned int vertex;
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	if (checkCompileErrors(vertex, "VERTEX"))
		glAttachShader(_ID, vertex);
	glDeleteShader(vertex);

	//if fragment shader is given, compile geometry shader
	if (fragmentShader != nullptr)
	{
		const char * fShaderCode = fragmentShader;
		unsigned int fragment;
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		if (checkCompileErrors(fragment, "FRAGMENT"))
			glAttachShader(_ID, fragment);
		glDeleteShader(fragment);
	}



	glLinkProgram(_ID);

	if (!checkProgramLinkingErrors())
		return false;
		
	return true;
}
bool Shader::initComputeShader(const char* computeShader) {
	if (computeShader == nullptr)
	{
		fprintf(stdout, "Null Compute Shader Code\n");
		return false;
	}
	// shader Program
	_ID = glCreateProgram();

	const char* cShaderCode = computeShader;

	// compute shader
	unsigned int computeS;
	computeS = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeS, 1, &cShaderCode, NULL);
	glCompileShader(computeS);
	if (checkCompileErrors(computeS, "COMPUTE"))
		glAttachShader(_ID, computeS);
	glDeleteShader(computeS);

	glLinkProgram(_ID);

	if (!checkProgramLinkingErrors())
		return false;

	return true;
}

GLint Shader::checkProgramLinkingErrors()
{
	GLint success;
	GLchar infoLog[1024];
	{
		glGetProgramiv(_ID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(_ID, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING of type: PROGRAM \n "<< infoLog<< std::endl;
		}
	}
	return success;
}


GLint Shader::checkCompileErrors(GLuint shader, const string &type)
{
	GLint success;
	GLchar infoLog[1024];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER_COMPILATION of type: PROGRAM \n " << type <<"\n" <<  infoLog << std::endl;
	}

	return success;
}


