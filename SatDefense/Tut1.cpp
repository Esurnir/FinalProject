#include "Utils.h"
#include "objLoader.h"
#include <gli/gli.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define WINDOW_TITLE_PREFIX "Chapter 4"

int
CurrentWidth = 800,
CurrentHeight = 600,
WindowHandle = 0;

unsigned FrameCount = 0;

GLuint
mvMatrixUniformLocation,
lDirUniformLocation,
NormalMatrixLocation,
texImageLocation,
triangleCount,
mvpMatrixUniformLocation,
eartTextureID,
BufferIds[3] = { 0 },
ShaderIds[3] = { 0 };

Matrix
ProjectionMatrix,
ViewMatrix,
ModelMatrix;

float CubeRotation = 0;
clock_t LastTime = 0;

void Initialize(int, char*[]);
void InitWindow(int, char*[]);
void ResizeFunction(int, int);
void RenderFunction(void);
void TimerFunction(int);
void IdleFunction(void);
void CreateCube(void);
void DestroyCube(void);
void DrawCube(void);
void CreateMesh(const char* filename);

int main(int argc, char* argv[])
{
	Initialize(argc, argv);

	glutMainLoop();

	exit(EXIT_SUCCESS);
}

void Initialize(int argc, char* argv[])
{
	GLenum GlewInitResult;

	InitWindow(argc, argv);

	glewExperimental = GL_TRUE;
	GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult) {
		fprintf(
			stderr,
			"ERROR: %s\n",
			glewGetErrorString(GlewInitResult)
			);
		exit(EXIT_FAILURE);
	}

	fprintf(
		stdout,
		"INFO: OpenGL Version: %s\n",
		glGetString(GL_VERSION)
		);

	glGetError();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	ExitOnGLError("ERROR: Could not set OpenGL depth testing options");

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	ExitOnGLError("ERROR: Could not set OpenGL culling options");

	ModelMatrix = IDENTITY_MATRIX;
	ProjectionMatrix = IDENTITY_MATRIX;
	ViewMatrix = IDENTITY_MATRIX;
	TranslateMatrix(&ViewMatrix, 0, 0, -2);

	CreateMesh("earth.obj");
}

void InitWindow(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
		);

	glutInitWindowSize(CurrentWidth, CurrentHeight);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE );

	WindowHandle = glutCreateWindow(WINDOW_TITLE_PREFIX);

	if (WindowHandle < 1) {
		fprintf(
			stderr,
			"ERROR: Could not create a new rendering window.\n"
			);
		exit(EXIT_FAILURE);
	}

	glutReshapeFunc(ResizeFunction);
	glutDisplayFunc(RenderFunction);
	glutIdleFunc(IdleFunction);
	glutTimerFunc(0, TimerFunction, 0);
	glutCloseFunc(DestroyCube);
	glEnable(GL_MULTISAMPLE_ARB);
}

void ResizeFunction(int Width, int Height)
{
	CurrentWidth = Width;
	CurrentHeight = Height;
	glViewport(0, 0, CurrentWidth, CurrentHeight);
}

void RenderFunction(void)
{
	++FrameCount;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawCube();

	glutSwapBuffers();
}

void IdleFunction(void)
{
	glutPostRedisplay();
}

void TimerFunction(int Value)
{
	if (0 != Value) {
		char* TempString = (char*)
			malloc(512 + strlen(WINDOW_TITLE_PREFIX));

		sprintf_s(
			TempString, 512 + strlen(WINDOW_TITLE_PREFIX),
			"%s: %d Frames Per Second @ %d x %d",
			WINDOW_TITLE_PREFIX,
			FrameCount * 4,
			CurrentWidth,
			CurrentHeight
			);

		glutSetWindowTitle(TempString);
		free(TempString);
	}

	FrameCount = 0;
	glutTimerFunc(250, TimerFunction, 1);
}

void CreateMesh(const char* filename)
{
	MeshVBO theMesh = meshLoad(filename);

	ShaderIds[0] = glCreateProgram();
	ExitOnGLError("ERROR: Could not create the shader program");
	{
		ShaderIds[1] = LoadShader("EarthShader.fragment.glsl", GL_FRAGMENT_SHADER);
		ShaderIds[2] = LoadShader("EarthShader.vertex.glsl", GL_VERTEX_SHADER);
		glAttachShader(ShaderIds[0], ShaderIds[1]);
		glAttachShader(ShaderIds[0], ShaderIds[2]);
	}
	glLinkProgram(ShaderIds[0]);
	ExitOnGLError("ERROR: Could not link the shader program");
	GLchar progStatus[500];
	GLsizei length;
	glGetProgramInfoLog(ShaderIds[0], 500, &length, progStatus);
	fprintf(stderr, "%s\n", progStatus);


	NormalMatrixLocation = glGetUniformLocation(ShaderIds[0], "NormalInvMatrix");
	lDirUniformLocation = glGetUniformLocation(ShaderIds[0], "lDir");
	texImageLocation = glGetUniformLocation(ShaderIds[0], "texImage");
	mvpMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "mvpMatrix");
	mvMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "mvMatrix");
	

	
	ExitOnGLError("ERROR: Could not get shader uniform locations");

	if (GLEW_ARB_get_program_binary) {
		const size_t MAX_SIZE = 1 << 24;
		char*  binary = new char[MAX_SIZE];
		GLenum format;
		GLint flength;
		glGetProgramBinary(ShaderIds[0], MAX_SIZE, &flength, &format, binary);
		std::ofstream binaryfile("bin.txt");
		binaryfile.write(binary, flength);
		delete[] binary;

	}
	else printf("No ARB_get_program_binary :(\n");
	glGenVertexArrays(1, &BufferIds[0]);
	ExitOnGLError("ERROR: Could not generate the VAO");
	glBindVertexArray(BufferIds[0]);
	ExitOnGLError("ERROR: Could not bind the VAO");

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	ExitOnGLError("ERROR: Could not enable vertex attributes");

	glGenBuffers(2, &BufferIds[1]);
	ExitOnGLError("ERROR: Could not generate the buffer objects");
	int nbuffersize;
	glBindBuffer(GL_ARRAY_BUFFER, BufferIds[1]);
	glBufferData(GL_ARRAY_BUFFER, theMesh.vertexBuffer.size()*sizeof(Vertex), &theMesh.vertexBuffer[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(struct Vertex,normal)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(struct Vertex, texCoord)));
	ExitOnGLError("ERROR: Could not bind the Vertex VBO to the VAO");
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &nbuffersize);
	printf("%d ", nbuffersize);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, theMesh.indexBuffer.size()*sizeof(unsigned int), &theMesh.indexBuffer[0], GL_STATIC_DRAW);

	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &nbuffersize);
	printf("%d ", nbuffersize);
/*
	vert = theMesh.Normals;
	glBindBuffer(GL_ARRAY_BUFFER, BufferIds[2]);
	glBufferData(GL_ARRAY_BUFFER, theMesh.Normals.size()*sizeof(Vec3), &vert[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	ExitOnGLError("ERROR: Could not bind the Normal VBO to the VAO");
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &nbuffersize);
	printf("%d ", nbuffersize);

	std::vector<Vec2> t = theMesh.texCoord;
	glBindBuffer(GL_ARRAY_BUFFER, BufferIds[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2)*theMesh.texCoord.size(), &t[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	ExitOnGLError("ERROR: Could not bind the Texture VBO to the VAO");
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &nbuffersize);
	printf("%d\n", nbuffersize);
*/
	ExitOnGLError("ERROR: Could not set VAO attributes");
	triangleCount = theMesh.indexBuffer.size();

	glBindVertexArray(0);

	ExitOnGLError("ERROR: Could not unbind Vertex");


	//glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &eartTextureID);

	ExitOnGLError("ERROR: Could not gen texture");
	
	gli::texture2D Texture(gli::load_dds("earth.dds"));
	assert(!Texture.empty());
	glBindTexture(GL_TEXTURE_2D, eartTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
	glTexStorage2D(GL_TEXTURE_2D,
		GLint(Texture.levels()),
		GLenum(gli::internal_format(Texture.format())),
		GLsizei(Texture.dimensions().x),
		GLsizei(Texture.dimensions().y));
	if (gli::is_compressed(Texture.format()))
	{
		for (gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glCompressedTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level),
				0, 0,
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				GLenum(gli::internal_format(Texture.format())),
				GLsizei(Texture[Level].size()),
				Texture[Level].data());
		}
	}
	else
	{
		for (gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level),
				0, 0,
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				GLenum(gli::external_format(Texture.format())),
				GLenum(gli::type_format(Texture.format())),
				Texture[Level].data());
		}
	}


}

void DestroyCube()
{
	glDetachShader(ShaderIds[0], ShaderIds[1]);
	glDetachShader(ShaderIds[0], ShaderIds[2]);
	glDeleteShader(ShaderIds[1]);
	glDeleteShader(ShaderIds[2]);
	glDeleteProgram(ShaderIds[0]);
	ExitOnGLError("ERROR: Could not destroy the shaders");

	glDeleteBuffers(2, &BufferIds[1]);
	glDeleteVertexArrays(1, &BufferIds[0]);
	ExitOnGLError("ERROR: Could not destroy the buffer objects");
}

void DrawCube(void)
{
	float CubeAngle;
	clock_t Now = clock();

	if (LastTime == 0)
		LastTime = Now;

	CubeRotation += 45.0f * ((float)(Now - LastTime) / CLOCKS_PER_SEC);
	CubeAngle = DegreesToRadians(CubeRotation);
	LastTime = Now;
	glm::mat4 modMatrix = glm::rotate(glm::mat4(),CubeAngle,glm::vec3(1.0f,0.0f,0.0f));
	modMatrix = glm::rotate(modMatrix, CubeAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -2.0f));
	glm::mat4 projMatrix = glm::perspective((float)(60*PI/180), ((float)CurrentWidth) / ((float)CurrentHeight), 1.0f, 100.0f);
	glm::mat4 mvp = projMatrix*viewMatrix*modMatrix;
	glm::mat4 mv = viewMatrix*modMatrix;

	
	glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(viewMatrix*modMatrix));// TransposeInverse3x3ModelView(&ModelMatrix, &ViewMatrix);

	glUseProgram(ShaderIds[0]);
	ExitOnGLError("ERROR: Could not use the shader program");

	
	glUniformMatrix3fv(NormalMatrixLocation, 1, GL_FALSE, &normalMatrix[0][0]);
	glUniformMatrix4fv(mvpMatrixUniformLocation, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(mvMatrixUniformLocation, 1, GL_FALSE, &mv[0][0]);
	glUniform3f(lDirUniformLocation, 1, 0, 0 );
	glUniform1i(texImageLocation, 0);
	ExitOnGLError("ERROR: Could not set the shader uniforms");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, eartTextureID);

	glBindVertexArray(BufferIds[0]);
	ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[2]);

	glDrawElements(GL_TRIANGLES, triangleCount, GL_UNSIGNED_INT,0);
	//glDrawArrays(GL_TRIANGLES, 0, triangleCount*3);
	//glutSolidTeapot(1);
	ExitOnGLError("ERROR: Could not draw the cube");

	glBindVertexArray(0);
	glUseProgram(0);
}
