#include "Camera.h"

#include <GLFW/glfw3.h>

struct World;

struct Player {
  Camera camera;
  bool _grounded = false;
  float _velocity_y = 0;
  enum class Mode { Survival, Creative, Menger };
  Mode _current_mode = Mode::Creative;

  void handleTick(const World& world);

  void jump() {
    if (_current_mode == Mode::Creative) {
      camera.translate(camera.up() * glm::vec3(0.2));
    }
  }

  void reset() {
    setPos(glm::vec3(20, 100, 20));
  }

  void moveDown() {
    if (_current_mode == Mode::Creative) {
      camera.translate(-camera.up() * glm::vec3(0.2));
    }
  }

  void move(int direction, World& world) {
    auto p = head();

    switch(direction) {
    case 0:  // right
      camera.strafe(-2); break;
    case 1:  // left
      camera.strafe(2); break;
    case 2:  // forward
      camera.translate(glm::vec3(2)*forward()); break;
    case 3:  // backward
      camera.translate(-glm::vec3(2)*forward()); break;
    }

    if (collided(world)) {
      setPos(p);
    }
  }

  glm::vec3 forward() const {
    glm::vec3 l = camera.look();
    return glm::normalize(glm::vec3(l.x, 0, l.z)) * Camera::zoom_speed;
  }

	void handleKey(int key, int scancode, int action, int mods) {
    // creative flying
    if (key == GLFW_KEY_F && action == GLFW_RELEASE) {
      if (_current_mode == Mode::Creative) {
        _current_mode = Mode::Survival;
      } else {
        _velocity_y = 0;
        _current_mode = Mode::Creative;
      }
    }

    // menger flying
    if (key == GLFW_KEY_F && action == GLFW_RELEASE && (mods & GLFW_MOD_CONTROL)) {
      if (_current_mode == Mode::Menger) {
        _current_mode = Mode::Survival;
      } else {
        _velocity_y = 0;
        _current_mode = Mode::Menger;
      }
    }

    if (_current_mode == Mode::Menger) {
      if (key == GLFW_KEY_W && action != GLFW_RELEASE) {
        camera.zoom(1);
      } else if (key == GLFW_KEY_S && action != GLFW_RELEASE) {
        camera.zoom(-1);
      } else if (key == GLFW_KEY_A && action != GLFW_RELEASE) {
        camera.strafe(-1);
      } else if (key == GLFW_KEY_D && action != GLFW_RELEASE) {
        camera.strafe(1);
      } else if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE) {
        camera.roll(-1);
      } else if (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE) {
        camera.roll(1);
      } else if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE) {
        camera.vertical(-1);
      } else if (key == GLFW_KEY_UP && action != GLFW_RELEASE) {
        camera.vertical(1);
      }
    }


    if (key == GLFW_KEY_SPACE && _grounded && action == GLFW_PRESS) {
      _grounded = false;
      _velocity_y = 0.2;
    }
  }


  bool grounded(const World& world);

  bool collided(const World& world);


  glm::vec3 feet() const {
    return camera.eye() - glm::vec3(0, 1.75, 0);
  }

  // gets the eye position 
  const glm::vec3& head() const {
    return camera.eye();
  }

  void setPos(glm::vec3 pos) {
    camera.setPos(pos);
  }

  glm::ivec3 blockPosition() const {
    return glm::floor(feet());
  }
};