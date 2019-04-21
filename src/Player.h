#include "Camera.h"

#include <GLFW/glfw3.h>
#include <string>

struct World;

struct Player {
  unsigned char _held_block = 0;
  Camera camera;
  bool _grounded = false;
  float _velocity_y = 0;
  enum class Mode { Survival, Creative };
  Mode _current_mode = Mode::Creative;

  void handleTick(const World& world);
  void handleMouse(int button, int action, int mods, World& world);

  void jump() {
    if (_current_mode == Mode::Creative) {
      camera.translate(camera.up() * glm::vec3(0.2));
    }
  }

  std::string modeString() {
    switch(_current_mode) {
    case Mode::Survival: return "Survival";
    case Mode::Creative: return "Creative";
    }
    return "Unknown Mode";
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
      camera.strafe(-1); break;
    case 1:  // left
      camera.strafe(1); break;
    case 2:  // forward
      camera.translate(glm::vec3(1)*forward()); break;
    case 3:  // backward
      camera.translate(-glm::vec3(1)*forward()); break;
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

    if (key == GLFW_KEY_SPACE && _grounded && action == GLFW_PRESS && _current_mode == Mode::Survival) {
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
    return glm::round(feet());
  }
};