#include "RenderWindow.h"
#include <iostream>

int main() {
  std::cout << "hello world" << std::endl;
  RenderWindow window {"Hello World", 800, 600};

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