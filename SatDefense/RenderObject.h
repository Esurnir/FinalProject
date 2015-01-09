#pragma once
#include <GL\glew.h>
#include "objLoader.h"
#include <unordered_map>

class RenderObject
{
public:
	RenderObject();
	~RenderObject();
	void loadMesh(char * filename);
	GLuint loadTexture(char * filename);
	GLuint loadProgram(char * vertex, char * fragment);
	GLuint loadProgram(char * vertex, char * fragment, char * tessControl, char * tessEval);
	void setUniformMatrix4fv(std::string name, float values[]);
	void setUniformMatrix3fv(std::string name, float values[]);
	void setUniform3fv(std::string name, float values[]);
	void setUniform1i(std::string name, int value);
	void setUniform1f(std::string name, float value);
	void activateProgram();
	void render();
	void bindTexture(GLuint textureIndex, GLenum textureUnit);
private:
	GLuint vao,program,type,vs,fs,tes,tcs;
	std::vector<GLuint> textureIDs,bufferIDs;
	std::unordered_map<std::string, GLuint> uniformLocations;
	int triangleCount;
};

