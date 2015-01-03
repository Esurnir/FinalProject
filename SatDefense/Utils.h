#ifndef UTILS_H
#define UTILS_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>
#include <time.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

static const double PI = 3.14159265358979323846;


struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

struct MeshVBO {
	std::vector<Vertex> vertexBuffer;
	std::vector<unsigned int> indexBuffer;
};

typedef struct Matrix
{
	float m[16];
} Matrix;

glm::mat3 TransposeInverse3x3ModelView(const Matrix* model, const Matrix* view);

extern const Matrix IDENTITY_MATRIX;

float Cotangent(float angle);
float DegreesToRadians(float degrees);
float RadiansToDegrees(float radians);


void ExitOnGLError(const char* error_message);
GLuint LoadShader(const char* filename, GLenum shader_type);

#endif
