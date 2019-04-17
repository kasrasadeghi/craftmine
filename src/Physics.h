#pragma once

#include <glm/glm.hpp>

namespace Physics {
  // checks if two boxes collide
  //   A is the first box's center
  //   pole0 is the base of the cylinder
  //   pole1 is the top
  //   assert (pole0 < pole1);
  bool verticalCollision(glm::vec3 A, float pole0, float pole1) {
    float top = A.y + 0.5;
    float bot = A.y - 0.5;
    return top > pole1 && bot < pole0;
  }

  bool horizontalCollision(glm::vec3 A, glm::vec3 pole, float radius) {
    return A.x + 0.5 > pole.x - radius && A.z + 0.5 > pole.z - radius
        && A.x - 0.5 < pole.x + radius && A.z + 0.5 < pole.z - radius;
  }
    
}