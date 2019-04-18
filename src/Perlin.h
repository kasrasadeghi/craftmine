#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "GlmHashes.h"
#include <stdio.h>

#include <iostream>
#include <unordered_map>
std::unordered_map<glm::ivec2, glm::vec2> gradients;

float TWO_PI = 6.28318530717958647692528676655900576;

float perlin(float x, float y) {
  auto rand2 = []() -> glm::vec2 {
    float theta = rand()/(float)RAND_MAX * TWO_PI;
    return glm::vec2 {glm::cos(theta), glm::sin(theta)};
  };

  auto get_gradient = [&](int i, int j) -> glm::vec2 {
    glm::ivec2 v {i, j}; 
    if (gradients.count(v) == 0) {
      gradients[v] = rand2();
    }
    return gradients[v];
  };

  glm::vec2 pos {x, y};

  // FIXME: handle negative values
  int x0 = (x > 0.0? (int)x: (int)x - 1);
  int x1 = x0 + 1;
  int y0 = (y > 0.0? (int)y: (int)y - 1);
  int y1 = y0 + 1;

  // gradients
  glm::vec2 g00 = get_gradient(x0, y0);
  glm::vec2 g01 = get_gradient(x0, y1);
  glm::vec2 g10 = get_gradient(x1, y0);
  glm::vec2 g11 = get_gradient(x1, y1);

  // corners
  glm::vec2 c00 {x0, y0};
  glm::vec2 c10 {x1, y0};
  glm::vec2 c01 {x0, y1};
  glm::vec2 c11 {x1, y1};

  // distances
  glm::vec2 d00 = pos - c00;
  glm::vec2 d01 = pos - c01;
  glm::vec2 d10 = pos - c10;
  glm::vec2 d11 = pos - c11;

  // products
  float p00 = glm::dot(g00, d00);
  float p01 = glm::dot(g01, d01);
  float p10 = glm::dot(g10, d10);
  float p11 = glm::dot(g11, d11);

  // heights and interpolation
  float h0_ = glm::mix(p00, p10, d00.x);
  float h1_ = glm::mix(p01, p11, d00.x);
  float h__ = glm::mix(h0_, h1_, d00.y);

  return h__;
}