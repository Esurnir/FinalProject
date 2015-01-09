#include "Utils.h"
#include "objLoader.h"
#include <gli/gli.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "RenderTextureFBO.h"
#include <iostream>
#include "RenderObject.h"

#define WINDOW_TITLE_PREFIX "Sattelite Defense"

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
nightImageLocation,
specImageLocation,
triangleCount,
gVPMatrixUniformLocation,
mMatrixUniformLocation,
gDispFactorUniformLocation,
gDisplacementMapUniformLocation,
ptSamplerUniformLocation,
dSamplerUniformLocation,
specularSamplerUniformLocation,
pixelOffsetUniformLocation,
sceneSamplerUniformLocation,
invNormalUniformLocation,
blurSamplerUniformLocation,
lightingUniformLocation,
nightUniformLocation,
athmosphereUniformLocation,
specbloomUniformLocation,
tessUniformLocation,
tex0Location,
eartTextureID[4] = { 0 }, //0 = diffuse 1 = night 2 = specular 3 = displacement
BufferIds[3] = { 0 }, //0 = VAO 1 = VBO 2 = VEB
quadIds[6] = { 0 },
ShaderIds[5] = { 0 },
blurShaderIds[3] = { 0 },
downSampleShaderIds[2] = { 0 },
compositShaderIds[2] = { 0 },
satShaderIds[3] = { 0 },
satBufferIds[3] = { 0 };
bool aamode = true, texture = true, animate = true, wireframe = 0,satOnly = false;
GLfloat speed = 0.5, anglex= 0, angley = 0,translatez = -2.5;
glm::mat4 viewMatrix;
glm::mat4 projMatrix;
glm::vec3 lDir;
int old_x = 0;
int old_y = 0;
int valid = 0;
float CubeAngle;

RenderObject * satellite;

int mode = 1;

#define DOWNSAMPLE_BUFFERS 2
#define BLUR_BUFFERS 2
RenderTexture *scene_buffer = 0, *downsample_buffer[DOWNSAMPLE_BUFFERS];
RenderTexture *blur_buffer[BLUR_BUFFERS];
RenderTexture *ms_buffer = 0;


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
void initQuad();
void drawQuad();
void destroyQuad();
void destroyFBOs();
void downSample(RenderTexture*, RenderTexture*, RenderTexture*);
void initBlurShader();
void blur(RenderTexture*, RenderTexture*, bool);
void keyboard(unsigned char, int, int);
void special(int, int, int);
void mouse_func(int button, int state, int x, int y);
void motion_func(int, int);
void initSatellite();
void initFBOs();
void renderSatellite();
void renderSatelliteAlone();

int main(int argc, char* argv[])
{
	Initialize(argc, argv);
	printf("Commands:\nmode [1-6]\n"
		"1: no lighting\n"
		"2: Basic Lighting\n"
		"3: Night Lights\n"
		"4: Athmosphere\n"
		"5: Specular Bloom\n"
		"6: Tesselation\n"
		"A: Anti Aliasing\n"
		"S: Sattelite only\n"
		"T: Texture RGB8/RGB16F\n"
		"+-:change animation speed\n"
		"R: reset\n"
		"W: Wireframe toggle\n"
		"Spacebar : start/stop animation\n"
		"Q or Escape to quit\n"
		"Arrow key : move camera\n"
		"click and drag to rotate camera arround\n");

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
	
	initFBOs();

	CreateMesh("earth.obj");
	initQuad();
	initBlurShader();
	initSatellite();
}

void InitWindow(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 2);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
		);

	glutInitWindowSize(CurrentWidth, CurrentHeight);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA );

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
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mouse_func);
	glutMotionFunc(motion_func);
	glutIdleFunc(IdleFunction);
	glutTimerFunc(0, TimerFunction, 0);
	glutCloseFunc(DestroyCube);
	//glEnable(GL_MULTISAMPLE_ARB);
}

void ResizeFunction(int Width, int Height)
{
	CurrentWidth = Width;
	CurrentHeight = Height;
	glViewport(0, 0, CurrentWidth, CurrentHeight);
	initFBOs();
}

void RenderFunction(void)
{
	++FrameCount;
	glViewport(0, 0, CurrentWidth, CurrentHeight);
	
	if (aamode) {
		ms_buffer->Activate();
		ExitOnGLError("Could not bind framebuffer");
		glEnable(GL_MULTISAMPLE);
	}
	else {
		scene_buffer->Activate();
		ExitOnGLError("Could not bind framebuffer");
		glDisable(GL_MULTISAMPLE);
	}
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, buffers);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!satOnly) {
		DrawCube();
		ExitOnGLError("ERROR: Problem after exiting cube");
		renderSatellite();
		ExitOnGLError("ERROR: Problem after exiting satellite");
	}
	else {
		renderSatelliteAlone();
	}
	if (aamode) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, scene_buffer->GetFramebuffer());
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "The frame buffer status is not complete!" << std::endl;
			exit(1);
		}

		ExitOnGLError("Could not bind draw buffer");
		
		glBlitFramebuffer(0, 0, CurrentWidth, CurrentHeight, 0, 0, CurrentWidth, CurrentHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		ExitOnGLError("Could not blit buffer");
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, CurrentWidth, CurrentHeight, 0, 0, CurrentWidth, CurrentHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0); //resetting default

	}/*
	scene_buffer->Bind();
	ExitOnGLError("Could not bind Read buffer");
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	ExitOnGLError("Could not bind back buffer");
	glBlitFramebuffer(0, 0, CurrentWidth, CurrentHeight , 0, 0, CurrentWidth, CurrentHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	ExitOnGLError("Could not blit the to screen");*/
	downSample(scene_buffer, downsample_buffer[1], downsample_buffer[0]);
	blur(downsample_buffer[1], blur_buffer[0], false);
	blur(blur_buffer[0], blur_buffer[1], true);
	

	glUseProgram(compositShaderIds[0]);
	glUniform1i(sceneSamplerUniformLocation, 0);
	glUniform1i(blurSamplerUniformLocation, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ExitOnGLError("Could not bind Read buffer");
	glActiveTexture(GL_TEXTURE0);
	scene_buffer->Bind();
	ExitOnGLError("Could not bind scene texture");
	glActiveTexture(GL_TEXTURE1);
	blur_buffer[1]->Bind();

	glViewport(0, 0, CurrentWidth, CurrentHeight);
	//glClear(GL_COLOR_BUFFER_BIT);
	drawQuad();

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
		ShaderIds[3] = LoadShader("EarthShader.TCS.glsl", GL_TESS_CONTROL_SHADER);
		ShaderIds[4] = LoadShader("EarthShader.TES.glsl", GL_TESS_EVALUATION_SHADER);
		glAttachShader(ShaderIds[0], ShaderIds[1]);
		glAttachShader(ShaderIds[0], ShaderIds[2]);
		glAttachShader(ShaderIds[0], ShaderIds[3]);
		glAttachShader(ShaderIds[0], ShaderIds[4]);
		GLint ret;
		CheckShader(ShaderIds[2], GL_COMPILE_STATUS, &ret, "unable to compile the vertex shader!");
		CheckShader(ShaderIds[1], GL_COMPILE_STATUS, &ret, "unable to compile the fragment shader!");
		CheckShader(ShaderIds[3], GL_COMPILE_STATUS, &ret, "unable to compile the TCS shader!");
		CheckShader(ShaderIds[4], GL_COMPILE_STATUS, &ret, "unable to compile the TES shader!");
	}
	glLinkProgram(ShaderIds[0]);
	ExitOnGLError("ERROR: Could not link the shader program");
	GLint ret;
	CheckShader(ShaderIds[0], GL_LINK_STATUS, &ret, "unable to link the program!");


	NormalMatrixLocation = glGetUniformLocation(ShaderIds[0], "NormalInvMatrix");
	lDirUniformLocation = glGetUniformLocation(ShaderIds[0], "lDir");
	texImageLocation = glGetUniformLocation(ShaderIds[0], "texImage");
	nightImageLocation = glGetUniformLocation(ShaderIds[0], "nightImage");
	specImageLocation = glGetUniformLocation(ShaderIds[0], "specImage");
	gVPMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "gVP");
	mMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "mMatrix");
	gDispFactorUniformLocation = glGetUniformLocation(ShaderIds[0], "gDispFactor");
	gDisplacementMapUniformLocation = glGetUniformLocation(ShaderIds[0], "gDisplacementMap");
	mvMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "mvMatrix");
	invNormalUniformLocation = glGetUniformLocation(ShaderIds[0], "invNormal");

	lightingUniformLocation = glGetUniformLocation(ShaderIds[0], "lighting");
	nightUniformLocation = glGetUniformLocation(ShaderIds[0], "night");
	athmosphereUniformLocation = glGetUniformLocation(ShaderIds[0], "athmosphere");
	specbloomUniformLocation = glGetUniformLocation(ShaderIds[0], "specbloom");
	tessUniformLocation = glGetUniformLocation(ShaderIds[0], "tess");
	

	
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

	ExitOnGLError("ERROR: Could not set VAO attributes");
	triangleCount = theMesh.indexBuffer.size();

	glBindVertexArray(0);

	ExitOnGLError("ERROR: Could not unbind Vertex");


	//glEnable(GL_TEXTURE_2D);
	glGenTextures(4, eartTextureID);

	ExitOnGLError("ERROR: Could not gen texture");

	float aniso = 0.0f;
	if (GLEW_EXT_texture_filter_anisotropic) glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	
	gli::texture2D Texture(gli::load_dds("earth-medium.dds"));
	if (Texture.empty())  {
		printf("Error loading earth.dds\n");
		exit(EXIT_FAILURE);
	}
	glBindTexture(GL_TEXTURE_2D, eartTextureID[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (GLEW_EXT_texture_filter_anisotropic) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
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


	Texture = gli::texture2D(gli::load_dds("night.dds"));

	if (Texture.empty())  {
		printf("Error loading night.dds\n");
		exit(EXIT_FAILURE);
	}
	glBindTexture(GL_TEXTURE_2D, eartTextureID[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (GLEW_EXT_texture_filter_anisotropic) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
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

	Texture = gli::texture2D(gli::load_dds("spec.dds"));

	if (Texture.empty())  {
		printf("Error loading spec.dds\n");
		exit(EXIT_FAILURE);
	}
	glBindTexture(GL_TEXTURE_2D, eartTextureID[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (GLEW_EXT_texture_filter_anisotropic) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
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

	Texture = gli::texture2D(gli::load_dds("displacement.dds"));

	if (Texture.empty())  {
		printf("Error loading displacement.dds\n");
		exit(EXIT_FAILURE);
	}
	glBindTexture(GL_TEXTURE_2D, eartTextureID[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (GLEW_EXT_texture_filter_anisotropic) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
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

	glDeleteTextures(3, eartTextureID);
	ExitOnGLError("ERROR: Could not destroy the texture objects");

	destroyFBOs();
	ExitOnGLError("ERROR: Could not destroy the FBOs");
	if (satellite) {
		delete satellite;
		satellite = NULL;
	}
}

void DrawCube(void)
{
	glEnable(GL_DEPTH_TEST);
	clock_t Now = clock();

	if (LastTime == 0)
		LastTime = Now;

	CubeRotation += animate ? 45.0f * ((float)(Now - LastTime) / CLOCKS_PER_SEC)*speed : 0;
	CubeAngle = DegreesToRadians(CubeRotation);
	LastTime = Now;
	glm::mat4 modMatrix = glm::rotate(glm::mat4(), CubeAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, translatez));

	viewMatrix = glm::rotate(viewMatrix, angley, glm::vec3(1.0f, 0.0f, 0.0f));
	viewMatrix = glm::rotate(viewMatrix, anglex, glm::vec3(0.0f, 1.0f, 0.0f));
	projMatrix = glm::perspective((float)(60*PI/180), ((float)CurrentWidth) / ((float)CurrentHeight), 1.0f, 10.0f);
	glm::mat4 vp = projMatrix*viewMatrix;
	glm::mat4 mv = viewMatrix*modMatrix;
	glm::vec4 lightDir = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	lightDir = viewMatrix*glm::rotate(glm::rotate(glm::mat4(), (float)(-23.4f*PI / 180.0f), glm::vec3(0.0, 0.0, 1.0f)), CubeAngle / (float)1.5, glm::vec3(0.0f, 1.0f, 0.0f))*lightDir;
	lDir = glm::vec3(lightDir);
	
	glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(viewMatrix*modMatrix));// TransposeInverse3x3ModelView(&ModelMatrix, &ViewMatrix);
	glm::vec3 dispfactor = glm::vec3(0.01);

	glUseProgram(ShaderIds[0]);
	ExitOnGLError("ERROR: Could not use the shader program");

	
	glUniformMatrix3fv(NormalMatrixLocation, 1, GL_FALSE, &normalMatrix[0][0]);
	glUniformMatrix3fv(invNormalUniformLocation, 1, GL_FALSE, &normalMatrix[0][0]);
	glUniformMatrix4fv(mMatrixUniformLocation, 1, GL_FALSE, &modMatrix[0][0]);
	glUniformMatrix4fv(mvMatrixUniformLocation, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix4fv(gVPMatrixUniformLocation, 1, GL_FALSE, &projMatrix[0][0]);
	glUniform3fv(gDispFactorUniformLocation, 1, &dispfactor[0]);
	glUniform1i(gDisplacementMapUniformLocation, 3);

	switch (mode) {
	case 1:
		glUniform1i(lightingUniformLocation, 0);
		glUniform1i(nightUniformLocation, 0);
		glUniform1i(athmosphereUniformLocation, 0);
		glUniform1i(specbloomUniformLocation, 0);
		glUniform1i(tessUniformLocation, 0);
		break;
	case 2:
		glUniform1i(lightingUniformLocation,1);
		glUniform1i(nightUniformLocation, 0);
		glUniform1i(athmosphereUniformLocation, 0);
		glUniform1i(specbloomUniformLocation, 0);
		glUniform1i(tessUniformLocation, 0);
		break;
	case 3:
		glUniform1i(lightingUniformLocation, 1);
		glUniform1i(nightUniformLocation, 1);
		glUniform1i(athmosphereUniformLocation, 0);
		glUniform1i(specbloomUniformLocation, 0);
		glUniform1i(tessUniformLocation, 0);
		break;
	case 4:
		glUniform1i(lightingUniformLocation, 1);
		glUniform1i(nightUniformLocation, 1);
		glUniform1i(athmosphereUniformLocation, 1);
		glUniform1i(specbloomUniformLocation, 0);
		glUniform1i(tessUniformLocation, 0);
		break;
	case 5:
		glUniform1i(lightingUniformLocation, 1);
		glUniform1i(nightUniformLocation, 1);
		glUniform1i(athmosphereUniformLocation, 1);
		glUniform1i(specbloomUniformLocation, 1);
		glUniform1i(tessUniformLocation, 0);
		break;
	case 6:
		glUniform1i(lightingUniformLocation, 1);
		glUniform1i(nightUniformLocation, 1);
		glUniform1i(athmosphereUniformLocation, 1);
		glUniform1i(specbloomUniformLocation, 1);
		glUniform1i(tessUniformLocation, 1);
		break;
	default:
		glUniform1i(lightingUniformLocation, 1);
		glUniform1i(nightUniformLocation, 1);
		glUniform1i(athmosphereUniformLocation, 1);
		glUniform1i(specbloomUniformLocation, 1);
		glUniform1i(tessUniformLocation, 1);
		break;
	}
	switch (wireframe) {
	case true:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case false:
		break;
	}

	glUniform3fv(lDirUniformLocation, 1, &lDir[0]);
	glUniform1i(texImageLocation, 0);
	glUniform1i(nightImageLocation, 1);
	glUniform1i(specImageLocation, 2);
	ExitOnGLError("ERROR: Could not set the shader uniforms");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, eartTextureID[0]);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, eartTextureID[1]);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, eartTextureID[2]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, eartTextureID[3]);

	glBindVertexArray(BufferIds[0]);
	ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[2]);

	//glDrawElements(GL_TRIANGLES, triangleCount, GL_UNSIGNED_INT,0);
	glDrawElements(GL_PATCHES, triangleCount, GL_UNSIGNED_INT, 0);
	//glDrawArrays(GL_TRIANGLES, 0, triangleCount*3);
	//glutSolidTeapot(1);
	ExitOnGLError("ERROR: Could not draw the cube");

	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void initFBOs() {
	GLenum format = texture ? GL_RGBA16F_ARB : GL_RGBA8;
	destroyFBOs();
	if (GLEW_EXT_framebuffer_blit && aamode) {
		ms_buffer = new RenderTexture(CurrentWidth, CurrentHeight, GL_TEXTURE_2D, 4, 16);
		ms_buffer->InitColor_RB(0, format);
		ms_buffer->InitColor_RB(1, format);
		ms_buffer->InitDepth_RB();
	}
	scene_buffer = new RenderTexture(CurrentWidth, CurrentHeight, GL_TEXTURE_2D);
	scene_buffer->InitColor_Tex(0, format);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	scene_buffer->InitColor_Tex(1, format);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	scene_buffer->InitDepth_RB();

	int w = CurrentWidth;
	int h = CurrentHeight;
	for (int i = 0; i<DOWNSAMPLE_BUFFERS; i++) {
		w /= 2;
		h /= 2;
		downsample_buffer[i] = new RenderTexture(w, h, GL_TEXTURE_2D);
		downsample_buffer[i]->InitColor_Tex(0, format);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	// blur pbuffers
	for (int i = 0; i<BLUR_BUFFERS; i++) {
		blur_buffer[i] = new RenderTexture(CurrentWidth / 4, CurrentHeight / 4, GL_TEXTURE_2D);
		blur_buffer[i]->InitColor_Tex(0, format);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
}

void destroyFBOs() {
	if (scene_buffer) {
		delete scene_buffer;
		scene_buffer = NULL;
	}
	for (int i = 0; i < DOWNSAMPLE_BUFFERS; i++) {
		if (downsample_buffer[i]) {
			delete downsample_buffer[i];
			downsample_buffer[i] = NULL;
		}
	}
	for (int i = 0; i<BLUR_BUFFERS; i++) {
		if (blur_buffer[i]){
			delete blur_buffer[i];
			blur_buffer[i] = NULL;
		}
	}
	if (ms_buffer) {
		delete ms_buffer;
		ms_buffer = NULL;
	}
}

void initQuad() {
	
	GLfloat quad[] = { -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0 }; //normalized trianglestrip
	GLfloat uv[] = { 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 }; //matching uv coordinate

	glGenVertexArrays(1, &quadIds[0]);
	ExitOnGLError("ERROR: Could not generate the VAO");
	glBindVertexArray(quadIds[0]);
	ExitOnGLError("ERROR: Could not bind the VAO");

	glGenBuffers(2, &quadIds[1]);
	ExitOnGLError("ERROR: Could not generate the buffer objects");

	glBindBuffer(GL_ARRAY_BUFFER, quadIds[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	ExitOnGLError("ERROR: Could not load quad attributes");

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	ExitOnGLError("ERROR: Could not enable vertex attributes");

	glBindBuffer(GL_ARRAY_BUFFER, quadIds[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uv), uv, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	quadIds[3] = glCreateProgram();
	ExitOnGLError("ERROR: Could not create the shader program");
	{
		quadIds[4] = LoadShader("passthrough.frag.glsl", GL_FRAGMENT_SHADER);
		quadIds[5] = LoadShader("passthrough.vert.glsl", GL_VERTEX_SHADER);

		GLint ret;
		CheckShader(quadIds[5], GL_COMPILE_STATUS, &ret, "unable to compile the vertex shader!");
		CheckShader(quadIds[4], GL_COMPILE_STATUS, &ret, "unable to compile the fragment shader!");

		glAttachShader(quadIds[3], quadIds[4]);
		glAttachShader(quadIds[3], quadIds[5]);
	}
	glLinkProgram(quadIds[3]);
	ExitOnGLError("ERROR: Could not link the shader program");
	GLint ret;
	CheckShader(quadIds[3], GL_LINK_STATUS, &ret, "unable to link the program!");
	if (GLEW_ARB_get_program_binary) {
		const size_t MAX_SIZE = 1 << 24;
		char*  binary = new char[MAX_SIZE];
		GLenum format;
		GLint flength;
		glGetProgramBinary(quadIds[3], MAX_SIZE, &flength, &format, binary);
		std::ofstream binaryfile("passthrough.txt");
		binaryfile.write(binary, flength);
		delete[] binary;

	}
	/*GLchar progStatus[500];
	GLsizei length;
	glGetProgramInfoLog(quadIds[3], 500, &length, progStatus);
	fprintf(stderr, "%s\n", progStatus);*/
	ptSamplerUniformLocation = glGetUniformLocation(quadIds[3], "ptSampler");

}

void deleteQuad() {
	glDeleteBuffers(2, &quadIds[1]);
	glDeleteVertexArrays(1, &quadIds[0]);
	ExitOnGLError("ERROR: Could not destroy the buffer objects");

	glDetachShader(quadIds[3], quadIds[4]);
	glDetachShader(quadIds[3], quadIds[5]);
	glDeleteShader(quadIds[4]);
	glDeleteShader(quadIds[5]);
	glDeleteProgram(quadIds[3]);
	ExitOnGLError("ERROR: Could not destroy the shaders");

}

void drawQuad() {
	glDisable(GL_DEPTH_TEST);
	//glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(quadIds[0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}



void downSample(RenderTexture* src, RenderTexture* dst,RenderTexture* tmp) {
	
	glUseProgram(quadIds[3]);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(ptSamplerUniformLocation, 0);
	src->Bind(1);
	tmp->Activate();
	glViewport(0, 0, CurrentWidth / 2, CurrentHeight / 2);
	drawQuad();
	glUseProgram(0);
	tmp->Deactivate();
	
	glUseProgram(quadIds[3]);
	glUniform1i(ptSamplerUniformLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	tmp->Bind();
	dst->Activate();
	glViewport(0, 0, CurrentWidth / 4, CurrentHeight / 4);
	drawQuad();
	glUseProgram(0);
	dst->Deactivate();
}

void initBlurShader() {
	blurShaderIds[0] = glCreateProgram();
	downSampleShaderIds[0] = glCreateProgram();
	compositShaderIds[0] = glCreateProgram();
	ExitOnGLError("ERROR: Could not create the shader program");
	{
		blurShaderIds[1] = LoadShader("passthrough.vert.glsl", GL_VERTEX_SHADER);
		blurShaderIds[2] = LoadShader("blur.frag.glsl", GL_FRAGMENT_SHADER);
		downSampleShaderIds[1] = LoadShader("alphaMultiply.frag.glsl", GL_FRAGMENT_SHADER);
		compositShaderIds[1] = LoadShader("composition.frag.glsl", GL_FRAGMENT_SHADER);

		GLint ret;
		CheckShader(blurShaderIds[1], GL_COMPILE_STATUS, &ret, "unable to compile the vertex shader!");
		CheckShader(blurShaderIds[2], GL_COMPILE_STATUS, &ret, "unable to compile the blur fragment shader!");
		CheckShader(downSampleShaderIds[1], GL_COMPILE_STATUS, &ret, "unable to compile the downsample fragment shader!");
		CheckShader(compositShaderIds[1], GL_COMPILE_STATUS, &ret, "unable to compile the composition fragment shader!");

		glAttachShader(blurShaderIds[0], blurShaderIds[1]);
		glAttachShader(blurShaderIds[0], blurShaderIds[2]);
		glAttachShader(downSampleShaderIds[0],blurShaderIds[1]);
		glAttachShader(downSampleShaderIds[0], downSampleShaderIds[1]);
		glAttachShader(compositShaderIds[0], blurShaderIds[1]);
		glAttachShader(compositShaderIds[0], compositShaderIds[1]);
	}
	glLinkProgram(blurShaderIds[0]);
	ExitOnGLError("ERROR: Could not link the blur shader program");
	glLinkProgram(downSampleShaderIds[0]);
	ExitOnGLError("ERROR: Could not link the blur shader program");
	glLinkProgram(compositShaderIds[0]);
	ExitOnGLError("ERROR: Could not link the composit shader program");
	GLint ret;
	CheckShader(blurShaderIds[0], GL_LINK_STATUS, &ret, "unable to link blur the program!");
	CheckShader(downSampleShaderIds[0], GL_LINK_STATUS, &ret, "unable to link the program!");
	if (GLEW_ARB_get_program_binary) {
		const size_t MAX_SIZE = 1 << 24;
		char*  binary = new char[MAX_SIZE];
		GLenum format;
		GLint flength;
		glGetProgramBinary(blurShaderIds[0], MAX_SIZE, &flength, &format, binary);
		std::ofstream binaryfile("blur.txt");
		binaryfile.write(binary, flength);
		delete[] binary;
	}
	pixelOffsetUniformLocation = glGetUniformLocation(blurShaderIds[0], "pixelOffset");
	tex0Location = glGetUniformLocation(blurShaderIds[0], "tex0");
	dSamplerUniformLocation = glGetUniformLocation(downSampleShaderIds[0], "dSampler");
	sceneSamplerUniformLocation = glGetUniformLocation(compositShaderIds[0], "sceneSampler");
	blurSamplerUniformLocation = glGetUniformLocation(compositShaderIds[0], "blurSampler");
	specularSamplerUniformLocation = glGetUniformLocation(downSampleShaderIds[0], "specularSampler");
}

void blur(RenderTexture* src, RenderTexture* dst,bool vertical) {
	glUseProgram(blurShaderIds[0]);
	src->Bind();
	glActiveTexture(GL_TEXTURE0);
	dst->Activate();
	glUniform1i(tex0Location, 0);
	glViewport(0, 0, (CurrentWidth / 4), (CurrentHeight / 4));
	float x_offset = 7*(1.0 / (2*CurrentWidth));
	float y_offset = 7*(1.0 / (2*CurrentHeight));
	glm::vec2 offset = vertical ? glm::vec2(0, y_offset) : glm::vec2(x_offset, 0);
	glUseProgram(blurShaderIds[0]);
	glUniform2fv(pixelOffsetUniformLocation,1,&offset[0]);
	glUniform1i(tex0Location, 0);
	drawQuad();
}

void keyboard(unsigned char key, int x, int y) {

	switch (key) {
	case 'q':
	case 27:
		exit(0);
		break;
	case '1': //no night
		mode = 1;
		break;
	case '2': //night
		mode = 2;
		break;
	case '3': //lighting
		mode = 3;
		break;
	case '4': //bloom
		mode = 4;
		break;
	case '5':
		mode = 5;
		break;
	case '6':
		mode = 6;
		break;
	case 'w':
		wireframe = !wireframe;
		break;
	case 'a':
		aamode = !aamode;
		initFBOs();
		break;
	case 't':
		texture = !texture;
		initFBOs();
		break;
	case 's':
		satOnly = !satOnly;
		break;
	case ' ':
		animate = !animate;
		break;
	case '-':
		speed = speed < 0.2 ? 0.1 : speed - 0.1;
		break;
	case '+':
		speed = speed > 2.9 ? 3.0 : speed + 0.1;
		break;
	case 'r':
		speed = 1.0;
		anglex = 0.0;
		angley = 0.0;
		translatez = -2.5;
		break;
	}

}



void special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		translatez += translatez >-0.2 ? 0.0: 0.1;
		break;
	case GLUT_KEY_DOWN:
		translatez -= 0.1;
		break;
	case GLUT_KEY_LEFT:
		anglex -= 0.0872664626;
		break;
	case GLUT_KEY_RIGHT:
		anglex += 0.0872664626;
		break;
	}
}



void mouse_func(int button, int state, int x, int y) {
	old_x = x;
	old_y = y;
	valid = state == GLUT_DOWN;
}

void motion_func(int x, int y) {
	if (valid) {
		int dx = old_x - x;
		int dy = old_y - y;
		anglex += 0.00872664626*dx;
		angley += 0.00872664626*dy;
	}
	old_x = x;
	old_y = y;
}

void initSatellite() {
	satellite = new RenderObject();
	satellite->loadMesh("carver.obj");
	satellite->loadTexture("carver.dds");
	satellite->loadTexture("carver-spec.dds");
	satellite->loadProgram("carver.vertex.glsl", "carver.fragment.glsl");
}

void renderSatellite() {
	glEnable(GL_DEPTH_TEST);
	satellite->activateProgram();
	satellite->bindTexture(0, 0);
	satellite->bindTexture(1, 1);
	switch (wireframe) {
	case true:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case false:
		break;
	}
	glm::mat4 modMatrix = glm::mat4(); // glm::rotate(glm::mat4(), CubeAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	modMatrix = glm::rotate(modMatrix, (float)(PI / 4), glm::vec3(0.0f, 1.0f, 0.0f));
	modMatrix = glm::rotate(modMatrix, CubeAngle/2, glm::vec3(1.0f, 0.0f, 0.0f));
	modMatrix = glm::translate(modMatrix, glm::vec3(0.0f, 0.0f, 1.3f));
	//modMatrix = glm::rotate(modMatrix, (float)(PI / 4), glm::vec3(0.0f, 1.0f, 0.0f));
	modMatrix = glm::rotate(modMatrix, (float)(PI / 2), glm::vec3(1.0f, 0.0f, 0.0f));
	
	modMatrix = glm::scale(modMatrix, glm::vec3(0.1f));
	glm::mat4 mv = viewMatrix*modMatrix;
	glm::mat4 mvp = projMatrix*viewMatrix*modMatrix;
	glm::mat3 norm = glm::inverseTranspose(glm::mat3(mv));

	satellite->setUniform3fv("lDir",&lDir[0]);
	satellite->setUniform1i("texImage",0);
	satellite->setUniform1i("specImage",1);
	satellite->setUniform1i("specbloom", mode == 5 || mode == 6);
	satellite->setUniformMatrix4fv("mvpMatrix", &mvp[0][0]);
	satellite->setUniformMatrix4fv("mvMatrix", &mv[0][0]);
	satellite->setUniformMatrix3fv("normalMatrix", &norm[0][0]);
	satellite->render();
	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
}

void renderSatelliteAlone() {
	glEnable(GL_DEPTH_TEST);
	satellite->activateProgram();
	satellite->bindTexture(0, 0);
	satellite->bindTexture(1, 1);
	switch (wireframe) {
	case true:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case false:
		break;
	}
	glm::mat4 modMatrix = glm::mat4(); // glm::rotate(glm::mat4(), CubeAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 vMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, translatez));

	vMatrix = glm::rotate(vMatrix, angley, glm::vec3(1.0f, 0.0f, 0.0f));
	vMatrix = glm::rotate(vMatrix, anglex, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 pMatrix = glm::perspective((float)(60 * PI / 180), ((float)CurrentWidth) / ((float)CurrentHeight), 1.0f, 10.0f);
	glm::vec4 lightDir = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	lightDir = viewMatrix*glm::rotate(glm::rotate(glm::mat4(), (float)(-23.4f*PI / 180.0f), glm::vec3(0.0, 0.0, 1.0f)), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f))*lightDir;
	glm::vec3 lir = glm::vec3(lightDir);
	glm::mat4 mv = vMatrix*modMatrix;
	glm::mat4 mvp = pMatrix*vMatrix*modMatrix;
	glm::mat3 norm = glm::inverseTranspose(glm::mat3(mv));

	satellite->setUniform3fv("lDir", &lir[0]);
	satellite->setUniform1i("texImage", 0);
	satellite->setUniform1i("specImage", 1);
	satellite->setUniform1i("specbloom", mode == 5 || mode == 6);
	satellite->setUniformMatrix4fv("mvpMatrix", &mvp[0][0]);
	satellite->setUniformMatrix4fv("mvMatrix", &mv[0][0]);
	satellite->setUniformMatrix3fv("normalMatrix", &norm[0][0]);
	satellite->render();
	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
}