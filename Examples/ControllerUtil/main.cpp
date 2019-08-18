#define GL_SILENCE_DEPRECATION

#include <iostream>

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>

SDL_Window* displayWindow;
bool gameIsRunning = true;

SDL_Joystick *playerOneController;

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    
    playerOneController = SDL_JoystickOpen(0);
    
    displayWindow = SDL_CreateWindow("Hello, World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    SDL_Event event;
    while (gameIsRunning) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                case SDL_WINDOWEVENT_CLOSE:
                    gameIsRunning = false;
                    break;
                    
                case SDL_JOYAXISMOTION:
                    if (abs(event.jaxis.value) < 5000) break;
                    std::cout << "Controller: " << event.jaxis.which << "\n";
                    std::cout << "Axis: " << unsigned(event.jaxis.axis) << "\n";
                    std::cout << "Value: " << event.jaxis.value << "\n";
                    std::cout << "\n";
                    break;
                    
                case SDL_JOYBUTTONDOWN:
                    std::cout << "Controller: " << event.jbutton.which << "\n";
                    std::cout << "Button: " << unsigned(event.jbutton.button) << "\n";
                    std::cout << "\n";
                    break;
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_JoystickClose(playerOneController);
    SDL_Quit();
    return 0;
}
