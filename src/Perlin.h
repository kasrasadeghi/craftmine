#pragma once

#include <noise/noise.h>

noise::module::Perlin gen;

float perlin(float x, float y, float z = 0) {
  return gen.GetValue(x, y, z) / 2.f + 0.5f;
}