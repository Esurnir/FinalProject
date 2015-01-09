#include "RenderObject.h"


RenderObject::RenderObject()
{
}


RenderObject::~RenderObject()
{
	if (type == 1) {
		glDetachShader(program, vs);
		glDetachShader(program, fs);
		glDeleteShader(vs);
		glDeleteShader(fs);
		glDeleteProgram(program);
	}
	else if (type == 2) {
		glDetachShader(program, vs);
		glDetachShader(program, fs);
		glDetachShader(program, tes);
		glDetachShader(program, tcs);
		glDeleteShader(vs);
		glDeleteShader(fs);
		glDeleteShader(tes);
		glDeleteShader(tcs);
		glDeleteProgram(program);
	}
	if (!bufferIDs.empty()) {
		glDeleteBuffers(bufferIDs.size(), &bufferIDs[0]);
		glDeleteVertexArrays(1, &vao);
	}
	if (!textureIDs.empty()) {
		glDeleteTextures(textureIDs.size(), &textureIDs[0]);
	}
}

void RenderObject::loadMesh(char * filename) {
	MeshVBO meshData = meshLoad(filename);
	glGenVertexArrays(1, &vao);
	ExitOnGLError("ERROR: Could not generate the VAO");
	glBindVertexArray(vao);
	ExitOnGLError("ERROR: Could not bind the VAO");

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	ExitOnGLError("ERROR: Could not enable vertex attributes");

	GLuint tmpbuf[2];
	glGenBuffers(2, &tmpbuf[0]);
	bufferIDs.assign(tmpbuf, tmpbuf + 2);
	ExitOnGLError("ERROR: Could not generate the buffer objects");
	int nbuffersize;
	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[0]);
	glBufferData(GL_ARRAY_BUFFER, meshData.vertexBuffer.size()*sizeof(Vertex), &meshData.vertexBuffer[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(struct Vertex, normal)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(struct Vertex, texCoord)));
	ExitOnGLError("ERROR: Could not bind the Vertex VBO to the VAO");
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &nbuffersize);
	printf("Vertex Buffer Data Size: %d ", nbuffersize);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIDs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData.indexBuffer.size()*sizeof(unsigned int), &meshData.indexBuffer[0], GL_STATIC_DRAW);

	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &nbuffersize);
	printf("Vertex Element Size: %d\n", nbuffersize);

	ExitOnGLError("ERROR: Could not set VAO attributes");
	triangleCount = meshData.indexBuffer.size();

	glBindVertexArray(0);

	ExitOnGLError("ERROR: Could not unbind Vertex");
}
GLuint RenderObject::loadTexture(char * filename) {
	GLuint texID = loadDDSTexture(filename, true);
	textureIDs.push_back(texID);
	return texID;
}
GLuint RenderObject::loadProgram(char * vertex, char * fragment) {
	program = glCreateProgram();
	ExitOnGLError("Could not create the RenderObject Shader Program");
	vs = LoadShader(vertex, GL_VERTEX_SHADER);
	fs = LoadShader(fragment, GL_FRAGMENT_SHADER);

	GLint ret;
	CheckShader(vs, GL_COMPILE_STATUS, &ret, "unable to compile the RenderObject vertex shader!");
	CheckShader(fs, GL_COMPILE_STATUS, &ret, "unable to compile the RenderObject fragment shader!");

	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	ExitOnGLError("ERROR: Could not link the RenderObject shader program");

	CheckShader(program, GL_LINK_STATUS, &ret, "unable to link blur the program!");
	type = 1;
	return program;
}
GLuint RenderObject::loadProgram(char * vertex, char * fragment, char * tessControl, char * tessEval) {
	program = glCreateProgram();
	ExitOnGLError("Could not create the RenderObject Shader Program");
	vs = LoadShader(vertex, GL_VERTEX_SHADER);
	fs = LoadShader(fragment, GL_FRAGMENT_SHADER);
	tcs = LoadShader(tessControl, GL_TESS_CONTROL_SHADER);
	tes = LoadShader(tessEval, GL_TESS_EVALUATION_SHADER);

	GLint ret;
	CheckShader(vs, GL_COMPILE_STATUS, &ret, "unable to compile the RenderObject vertex shader!");
	CheckShader(fs, GL_COMPILE_STATUS, &ret, "unable to compile the RenderObject fragment shader!");
	CheckShader(tcs, GL_COMPILE_STATUS, &ret, "unable to compile the RenderObject tesselation control shader!");
	CheckShader(tes, GL_COMPILE_STATUS, &ret, "unable to compile the RenderObject tesselation fragment shader!");

	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	ExitOnGLError("ERROR: Could not link the RenderObject shader program");

	CheckShader(program, GL_LINK_STATUS, &ret, "unable to link blur the program!");
	type = 2;
	return program;
}
void RenderObject::setUniformMatrix4fv(std::string name, float values[]) {
	std::unordered_map<std::string, GLuint>::const_iterator it = uniformLocations.find(name);
	if (it == uniformLocations.end()) {
		uniformLocations[name] = glGetUniformLocation(program, name.c_str());
		ExitOnGLError("Could not find Uniform Location in RenderObject Program");
	}
	glUniformMatrix4fv(uniformLocations[name], 1, GL_FALSE, values);
}
void RenderObject::setUniformMatrix3fv(std::string name, float values[]) {
	std::unordered_map<std::string, GLuint>::const_iterator it = uniformLocations.find(name);
	if (it == uniformLocations.end()) {
		uniformLocations[name] = glGetUniformLocation(program, name.c_str());
		ExitOnGLError("Could not find Uniform Location in RenderObject Program");
	}
	glUniformMatrix3fv(uniformLocations[name], 1, GL_FALSE, values);
}
void RenderObject::setUniform3fv(std::string name, float values[]) {
	std::unordered_map<std::string, GLuint>::const_iterator it = uniformLocations.find(name);
	if (it == uniformLocations.end()) {
		uniformLocations[name] = glGetUniformLocation(program, name.c_str());
		ExitOnGLError("Could not find Uniform Location in RenderObject Program");
	}
	glUniform3fv(uniformLocations[name], 1, values);
}
void RenderObject::setUniform1i(std::string name, int value) {
	std::unordered_map<std::string, GLuint>::const_iterator it = uniformLocations.find(name);
	if (it == uniformLocations.end()) {
		uniformLocations[name] = glGetUniformLocation(program, name.c_str());
		ExitOnGLError("Could not find Uniform Location in RenderObject Program");
	}
	glUniform1i(uniformLocations[name], value);
}
void RenderObject::setUniform1f(std::string name, float value) {
	std::unordered_map<std::string, GLuint>::const_iterator it = uniformLocations.find(name);
	if (it == uniformLocations.end()) {
		uniformLocations[name] = glGetUniformLocation(program, name.c_str());
		ExitOnGLError("Could not find Uniform Location in RenderObject Program");
	}
	glUniform1f(uniformLocations[name], value);
}
void RenderObject::activateProgram() {
	glUseProgram(program);
}
void RenderObject::render() {
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIDs[1]);
	type == 1 ? glDrawElements(GL_TRIANGLES, triangleCount, GL_UNSIGNED_INT, 0) : glDrawElements(GL_PATCHES, triangleCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
void RenderObject::bindTexture(GLuint textureIndex, GLenum textureUnit) {
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, textureIDs[textureIndex]);
}