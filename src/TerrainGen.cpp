#include "TerrainGen.h"
#include "World.h"
#include "Player.h"
#include "Perlin.h"
#include "Terrain.h"

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

  auto octave = [](int h, bool v){
    using namespace Terrain;
    if (h < 50 && v) { return STONE; }
    if (h < 55 && v) { return DIRT; }
    if (v)           { return GRASS; }
    if (h < 45 && not v) { return WATER; }
    return AIR;
  };

  // FIXME: is everything in this function actually in the same chunk?

  for (int i = 0; i < CHUNK_SIZE; ++i)
  for (int k = 0; k < CHUNK_SIZE; ++k) {
    int h = 50 + perlin((bi + i) / 50.f, (bk + k) / 50.f) * 30;
    h = glm::clamp(h, 0, 127);

    for (int j = 0; j < 80; ++j) {
      world(bi + i, j, bk + k) = octave(j, j <= h);
    }
  }

  world._chunks[chunk_index].generated = true;
}