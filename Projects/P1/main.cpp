#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>

SDL_Window* displayWindow;
bool gameIsRunning = true;

std::vector<ShaderProgram> program;
std::vector<glm::mat4> modelMatrix;
glm::mat4 viewMatrix, projectionMatrix;

std::vector<GLuint> playerTextureIDs;



GLuint LoadTexture(const char* filePath) {
	int w, h, n;
	unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
	       
	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(image);
	return textureID;
}

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Textured", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 480);

	//load shaders to each program
	ShaderProgram prog;
	prog.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
	program.push_back(prog);

	ShaderProgram prog2;
	prog2.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
	program.push_back(prog2);

	ShaderProgram prog3;
	prog3.Load("shaders/vertex.glsl", "shaders/fragment.glsl");
	program.push_back(prog3);

	//load textured assets
	GLuint playerTextureID1 = LoadTexture("baseball.png");
	GLuint playerTextureID2 = LoadTexture("basketball.png");

	//append them to vector
	playerTextureIDs.push_back(playerTextureID1);
	playerTextureIDs.push_back(playerTextureID2);

	//view matrix and projection matrix
	viewMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

	//modelMatrix = glm::mat4(1.0f);
	for (auto i = 0; i < playerTextureIDs.size(); i++) {
		modelMatrix.push_back(glm::mat4(1.0f));
	}

	//push back untextured model
	modelMatrix.push_back(glm::mat4(1.0f));

	//program 1
	program[0].SetProjectionMatrix(projectionMatrix);
	program[0].SetViewMatrix(viewMatrix);
	program[0].SetColor(1.0f, 0.0f, 0.0f, 1.0f);

	//program 2
	program[1].SetProjectionMatrix(projectionMatrix);
	program[1].SetViewMatrix(viewMatrix);
	program[1].SetColor(1.0f, 0.0f, 0.0f, 1.0f);

	//program 3
	program[2].SetProjectionMatrix(projectionMatrix);
	program[2].SetViewMatrix(viewMatrix);
	program[2].SetColor(1.0f, 0.0f, 0.0f, 1.0f);

	//use all programs
	glUseProgram(program[0].programID);
	glUseProgram(program[1].programID);
	glUseProgram(program[2].programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void ProcessInput() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			gameIsRunning = false;
		}
	}
}

float lastTicks = 0;
float baseball_travel = 0;
float basketball_rotate = 0;

void Update() {
	//sync
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	//update baseball
	baseball_travel += 1.0f * deltaTime;
	modelMatrix[0] = glm::mat4(1.0f);
	modelMatrix[0] = glm::translate(modelMatrix[0], glm::vec3(baseball_travel, 0.0f, 0.0f));

	//update basketball
	basketball_rotate += 45.0 * deltaTime;
	modelMatrix[1] = glm::mat4(1.0f);
	modelMatrix[1] = glm::rotate(modelMatrix[1],
		glm::radians(basketball_rotate),
		glm::vec3(0.0f, 0.0f, 1.0f));

	//update triangle
	modelMatrix[2] = glm::mat4(1.0f);
	modelMatrix[2] = glm::translate(modelMatrix[2], glm::vec3(-1.0f, 1.0f, 0.0f));
}

void Render() {

	glClear(GL_COLOR_BUFFER_BIT);

	//render each element
	for (auto i = 0; i < program.size(); i++) {

		program[i].SetModelMatrix(modelMatrix[i]);
		ShaderProgram curr_prog = program[i];

		if (i != program.size() - 1) {

			float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
			float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

			glBindTexture(GL_TEXTURE_2D, playerTextureIDs[i]);

			glVertexAttribPointer(curr_prog.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(curr_prog.positionAttribute);

			glVertexAttribPointer(curr_prog.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
			glEnableVertexAttribArray(curr_prog.texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(curr_prog.positionAttribute);
			glDisableVertexAttribArray(curr_prog.texCoordAttribute);
		}

		else {
			float vertices[] = { 0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f };
			glVertexAttribPointer(curr_prog.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(curr_prog.positionAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			glDisableVertexAttribArray(curr_prog.positionAttribute);
		}
	}

	SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	Initialize();

	while (gameIsRunning) {
		ProcessInput();
		Update();
		Render();
	}

	Shutdown();
	return 0;
}
