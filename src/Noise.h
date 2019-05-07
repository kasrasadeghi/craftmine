#pragma once

#include <SimplexNoise.h>
#include <noise/noise.h>

inline float perlin(float x, float y, float z = 0) {
  static noise::module::Perlin gen;
  return gen.GetValue(x, y, z) / 2.f + 0.5f;
}

inline float simplex(float x, float y, float z = 0) {
  return SimplexNoise::noise(x, y, z) / 2 + 0.5;
}
