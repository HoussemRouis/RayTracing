#ifndef SHADER_H
#define SHADER_H



#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <string>

class Shader
{
public:
	unsigned int _ID{};

	//Init Shaders from string code
	bool init(const char* vertexShader, const char* fragmentShader = nullptr);
	bool initComputeShader(const char* computeShader);

	//Use the Shader
	void use()
	{
		glUseProgram(_ID);
	}
	GLuint getID(){ return _ID; }
	//Set an int input
	void setInt(const std::string &name, int value) const
	{
		glUniform1i(glGetUniformLocation(_ID, name.c_str()), value);
	}

	//Set a float input
	void setFloat(const std::string &name, float value) const
	{
		glUniform1f(glGetUniformLocation(_ID, name.c_str()), value);
	}


	//Set a vec3 input
	void setVec3(const std::string &name, const glm::vec3 &vec) const
	{
		glUniform3fv(glGetUniformLocation(_ID, name.c_str()), 1, &vec[0]);
	}

	//Set a vec4 input
	void setVec4(const std::string &name, const glm::vec4 &vec) const
	{
		glUniform4fv(glGetUniformLocation(_ID, name.c_str()), 1, &vec[0]);
	}
	//Set a mat4 input
	void setMat4(const std::string &name, const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(_ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	//Check linking Errors
	GLint checkProgramLinkingErrors();
private:
	// utility function for checking shader compilation/linking errors.
	GLint checkCompileErrors(GLuint shader, const std::string & type);
};


#endif