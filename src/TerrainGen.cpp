#include "TerrainGen.h"
#include "World.h"
#include "Player.h"
#include "Perlin.h"
#include "Terrain.h"

#include <iostream>

// FIXME: is every chunk actually only loaded once?

void TerrainGen::spawn(World& world, Player& player) {
  auto chunk_index = World::toChunk(player.blockPosition());

  for (int i = -3; i <= 3; ++i) {
    for (int k = -3; k <= 3; ++k) {
      glm::ivec2 curr_index = chunk_index + glm::ivec2(i, k);
      world._chunks[curr_index] = {};
      chunk(world, curr_index);
    }
  }
  for (auto& p : world._chunks) {
    p.second.generated = true;
  }
}

void TerrainGen::chunk(World& world, glm::ivec2 chunk_index) {
  assert (not world._chunks.at(chunk_index).generated);

  int bi = chunk_index.x * CHUNK_SIZE;
  int bk = chunk_index.y * CHUNK_SIZE;

  auto octave = [](int h, bool v) {
    using namespace Terrain;
    if (h < 50 && v) { return STONE; }
    if (h < 55 && v) { return DIRT; }
    if (v)           { return GRASS; }
    if (h < 45 && not v) { return AIR; }
    return AIR;
  };

  auto stretch_octave = [](int s) {
    using namespace Terrain;
    if (s < 2) return GRASS;
    if (s < 8) return DIRT;
    return STONE;
  };

  // FIXME: is everything in this function actually in the same chunk?

  for (int i = 0; i < CHUNK_SIZE; ++i)
  for (int k = 0; k < CHUNK_SIZE; ++k)
  {
    int base_y = 30;
    float delta_y = 0;
    delta_y += 10 * perlin((bi + i) / 64.f, (bk + k) / 64.f);
    delta_y += 20 * perlin((bi + i) / 32.f, (bk + k) / 32.f);

    delta_y = glm::pow(delta_y, 1.15f);

    int h = glm::clamp<int>(base_y + delta_y, 0, 127);

    // solidity of block
    bool column[128] = {}; // all elements zero
    for (int j = 0; j < 80; ++j) {
      column[j] = j <= h;
    }

    for (int j = 30; j < 100; ++j) {
      if (not column[j]) {
        float p = 4 * perlin((bi + i) / 100.f, j / 50.f, (bk + k) / 100.f);
        float k = (128 - j)/128.f;
        column[j] = glm::floor(k * p);
      }
    }
    
    int stretch = 0;
    bool seen = false;
    for (int j = 127; j >= 0; --j) {
      if (column[j]) {
        seen = true;
        world(bi + i, j, bk + k) = stretch_octave(stretch);
        stretch ++;
      } else if (seen) {
        stretch = 1;
      } else {
        stretch = 0;
      }
    }


  }

  world._chunks[chunk_index].generated = true;
}