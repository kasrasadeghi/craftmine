#include "TerrainGen.h"
#include "World.h"
#include "Player.h"
#include "Perlin.h"

void TerrainGen::spawn(World& world, Player& player, int width, int height) {
  for (int i = -width/2; i < width/2; ++i) {
    for (int k = -height/2; k < height/2; ++k) {
      int h = 50 + perlin(i / 50.f, k / 50.f) * 10;
      glm::ivec3 p = player.blockPosition() + glm::ivec3(i, h, k);

      world(p.x, p.y,   p.z) = 1;
      world(p.x, p.y-1, p.z) = 1;
      world(p.x, p.y-2, p.z) = 1;
    }
  }
}

void TerrainGen::chunk(World& world, glm::ivec2 chunk_index) {
  int bi = chunk_index.x * CHUNK_SIZE;
  int bk = chunk_index.y * CHUNK_SIZE;

  for (int i = 0; i < 16; ++i)
  for (int k = 0; k < 16; ++k) {
    int h = 50 + perlin(i / 50.f, k / 50.f) * 10;
    world(bi + i, h,   bk + k) = 1;
    world(bi + i, h-1, bk + k) = 1;
    world(bi + i, h-2, bk + k) = 1;
  }
}