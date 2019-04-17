#include "Camera.h"
#include <GLFW/glfw3.h>

struct Player {
  Camera camera;
  bool _grounded = false;
  float _velocity_y;

  void handleTick() {
    // generate impulse to jerk player if collided
    // if movement resolves collision, no impulse
    // otherwise impulse

    // if not on ground, tick gravity
    // if (not _grounded) {
      // tick gravity
      // _velocity_y -= 0.5;

      // apply vel_y

      // if now grounded, vel_y = 0
    // }
    

  }
	void handleKey(int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE) {
      return;
    }
  }
};