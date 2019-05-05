#include "TerrainGen.h"
#include "World.h"
#include "Player.h"
#include "Perlin.h"
#include "Terrain.h"

#include <iostream>

// FIXME: is every chunk actually only loaded once?

void TerrainGen::spawn(World& world, Player& player) {
  auto chunk_index = World::toChunk(player.blockPosition());

  for (int i = -RENDER_DISTANCE; i <= RENDER_DISTANCE; ++i) {
    for (int k = -RENDER_DISTANCE; k <= RENDER_DISTANCE; ++k) {
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

  for (int di = 0; di < CHUNK_SIZE; ++di)
  for (int dk = 0; dk < CHUNK_SIZE; ++dk)
  {
    int i = bi + di;
    int k = bk + dk;

    // solidity of block
    bool column[128] = {}; // all elements zero
    int highest_block = 40; // sea level

    int height_base = 10;
    // int height_base = 30 + 50 * perlin(i/ 500.f, k / 500.f);

    // column[height_base] = 1;

    // int base_y = height_base;
    // float delta_y = 0;
    // delta_y += 10 * perlin(i / 150.f, k / 150.f);
    // delta_y = glm::pow(delta_y, 1.15f);
    // int h = glm::clamp<int>(base_y + delta_y, 0, 127);

    // for (int j = 0; j < 80; ++j) {
    //   column[j] = j <= h;
    // }

    // float vertical_variance = 50 + 20 * perlin(i/70.f, k/70.f);

    for (int y = 0; y < 128; ++y) {
      if (not column[y]) {

        // float p = perlin(i / 150.f, y / 128.f, k / 150.f);
        // float scale1 = glm::exp(-0.1 * j + 3/20.f;
        // float scale = glm::mix(scale0, scale1, 0.2);
        
        // column[y] = glm::floor(scale0 * p);
        // column[y] = glm::floor(scale0);
        // column[y] = glm::floor(p);

        // float p2 = perlin((i + 100) / 600.f, (k+ 100) / 600.f);

        
        // float gradient =  1 + 1/p2 - y/64.f;
        float p2 = perlin(i / 150.f, 0, k / 150.f);
        float p = perlin(i / 150.f, y / 128.f, k / 150.f);

        float scalefac = .4f + .4f * p2;
        float gradient = (2 + p2) - y/64.f;

        column[y] = glm::floor(glm::mix(gradient, p, scalefac));
        highest_block = column[y] ? glm::max<int>(y, highest_block) : highest_block;
        // column[y] = glm::floor(scale0);
      }
    }
    
    int stretch = 0;
    bool seen = false;
    for (int j = 127; j >= 0; --j) {
      if (column[j]) {
        seen = true;
        world(i, j, k) = stretch_octave(stretch);
        stretch ++;
      } else {
        if (seen) {
          stretch = 2;
        } else {
          stretch = 0;
        }
        if (j < 40) {
          world(i, j, k) = Terrain::WATER;
        }
      }
    }
    // add trees!

    // see if this column needs a tree
    float p = perlin(i / 10.f, k / 10.f);
    constexpr float tree_thresh = 0.98f;
 
    bool make_tree = 
        p >= tree_thresh 
        && world(i, highest_block, k) == Terrain::GRASS;

    if (make_tree) {
      // calculate height
      float diff = (p - tree_thresh) * 100;
      
      // bounds check ehh not really importaint but you kno
      // diff = glm::min(128 - highest_block, diff);

      // plant a tree at highest_block
      for (int dj = 0; dj < diff; ++dj) {
        int j = dj + highest_block + 1;
        if (j < CHUNK_HEIGHT) {
          world(i, j, k) = Terrain::DIRT;
        }
      }
    }
  }

  world._chunks[chunk_index].generated = true;
}