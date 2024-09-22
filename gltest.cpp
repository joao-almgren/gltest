#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "ascii_keys.h"
using namespace std;
using namespace glm;

int gWidth = 800, gHeight = 600;
GLuint gVBO, gVAO, gIBO, gProgram;

const char* loadShaderSource(const char* filePath)
{
	ifstream file(filePath);
	if (!file.is_open())
	{
		cerr << "Could not open shader file: " << filePath << endl;
		return nullptr;
	}

	stringstream buffer;
	buffer << file.rdbuf();
	string source = buffer.str();

	char* sourceCStr = new char[source.size() + 1];
	std::copy(source.begin(), source.end(), sourceCStr);
	sourceCStr[source.size()] = 0;

	return sourceCStr;
}

GLuint compileShader(GLenum type, const char* shaderSource)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		cerr << "Error compiling shader: " << infoLog << endl;
	}

	return shader;
}

GLuint createShaderProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename)
{
	const char* vertexShaderSource = loadShaderSource(vertexShaderFilename);
	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	delete[] vertexShaderSource;

	const char* fragmentShaderSource = loadShaderSource(fragmentShaderFilename);
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	delete[] fragmentShaderSource;

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}

void initialize()
{
	float vertices[] =
	{
		-1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.0f
	};

	unsigned int indices[] =
	{
		2, 6, 7,
		2, 3, 7,
		0, 4, 5,
		0, 1, 5,
		0, 2, 6,
		0, 4, 6,
		1, 3, 7,
		1, 5, 7,
		0, 2, 3,
		0, 1, 3,
		4, 6, 7,
		4, 5, 7
	};

	glGenVertexArrays(1, &gVAO);
	glGenBuffers(1, &gVBO);
	glGenBuffers(1, &gIBO);

	glBindVertexArray(gVAO);

	glBindBuffer(GL_ARRAY_BUFFER, gVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	gProgram = createShaderProgram("vs.glsl", "fs.glsl");
}

void checkGLError()
{
	for (auto err = glGetError(); err != GL_NO_ERROR; err = glGetError())
		cerr << "OpenGL error: " << err << endl;
}

void handleKeypress(unsigned char key, int x, int y)
{
	switch (key)
	{
		case ASCII_SPACE:
			break;
		case ASCII_ESCAPE:
			glutLeaveMainLoop();
			break;
	}
}

void handleSpecialKeypress(int key, int x, int y)
{
    switch (key)
	{
		case GLUT_KEY_LEFT:
			break;
    }
}

void display()
{
	static float gAngle = 0.0f;
	gAngle += radians(1.0f);
	if (gAngle > 2.0f * M_PI)
		gAngle -= 2.0f * M_PI;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 projection = perspective(radians(60.0f), (float)gWidth / (float)gHeight, 0.1f, 100.0f);
	mat4 view = lookAt(vec3(0.0f, 0.0f, 5.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 model = mat4(1.0f);
	model = rotate(model, gAngle, vec3(0.0f, 1.0f, 0.0f));
	model = rotate(model, gAngle * 2.0f, vec3(1.0f, 0.0f, 0.0f));
	mat4 mvp = projection * view * model;

	glUseProgram(gProgram);
	GLint mvpLocation = glGetUniformLocation(gProgram, "mvp");
	glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, value_ptr(mvp));

	glBindVertexArray(gVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	checkGLError();
	glFlush();
	glutSwapBuffers();
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	gWidth = width;
	gHeight = height;
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitContextVersion(3, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitWindowSize(gWidth, gHeight);
	glutCreateWindow("gltest");

	cerr << "Driver: " << glGetString(GL_VENDOR) << " " << glGetString(GL_VERSION) << endl;

	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	initialize();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(display);
	glutKeyboardFunc(handleKeypress);
	glutSpecialFunc(handleSpecialKeypress);
	glutMainLoop();

	glDeleteVertexArrays(1, &gVAO);
	glDeleteBuffers(1, &gVBO);
	glDeleteBuffers(1, &gIBO);
	glDeleteProgram(gProgram);

	return 0;
}
