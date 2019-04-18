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
    return not(top < pole0 || bot > pole1);
  }

  bool horizontalCollision(glm::vec3 A, glm::vec3 pole, float radius) {
    return A.x + 0.5 > pole.x - radius && A.z + 0.5 > pole.z - radius
        && A.x - 0.5 < pole.x + radius && A.z - 0.5 < pole.z + radius;
  }
  
  // line points away from the center of the box
  glm::vec2 lineBoxIntersection(glm::vec2 center, glm::vec2 line) {

    float tx = 100;
    float ty = 100;
    if (line.x != 0) {
      tx = glm::abs(0.5 / line.x);
    }
    if (line.y != 0) {
      ty = glm::abs(0.5 / line.y);
    }

    float t = glm::min(tx, ty);
    if (t != 100) {
      return center + line * glm::vec2(t);
    }
    return glm::vec2(-1);
  }

  glm::vec2 boxCircleOverlap(glm::vec2 center, glm::vec2 pole, float radius) {
    glm::vec2 line = pole - center;

    glm::vec2 box_isect = lineBoxIntersection(center, line);
    
    glm::vec2 circle_isect = center + glm::normalize(line) * (glm::length(line) - radius);

    return box_isect - circle_isect;
  }
}