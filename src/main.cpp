#include "RenderWindow.h"
#include <iostream>

int main() {
  std::cout << "hello world" << std::endl;
  RenderWindow window {"Hello World"};

  window.setKeyCallback([&](int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE) {
      window.close();
    }
  });

  while (window.isOpen()) {
    window.swapBuffers();
    glfwPollEvents();
  }
}