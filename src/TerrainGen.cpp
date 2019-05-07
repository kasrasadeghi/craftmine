#include "TerrainGen.h"
#include "World.h"
#include "Player.h"
#include "Perlin.h"
#include "Terrain.h"

#include <iostream>
#include <glm/gtx/string_cast.hpp>

// FIXME: is every chunk actually only loaded once?

void TerrainGen::spawn(World& world, Player& player) {
  auto chunk_index = World::toChunk(player.blockPosition());

  for (int i = -RENDER_DISTANCE; i <= RENDER_DISTANCE; ++i) {
    for (int k = -RENDER_DISTANCE; k <= RENDER_DISTANCE; ++k) {
      glm::ivec2 curr_index = chunk_index + glm::ivec2(i, k);
      world._chunks.emplace(curr_index, Chunk());
      chunk(world, curr_index);
    }
  }
}

void TerrainGen::chunk(World& world, glm::ivec2 chunk_index) {
  assert (world.hasChunk(chunk_index));
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
  }
  
  struct Tree_ {
    glm::ivec2 pos;
    float size; // 0 .. 3
  };

  // decide where to put trees
  std::vector<Tree_> trees;

  auto rand1 = []() -> float {
    return rand()/(float)RAND_MAX;
  };

  auto circle_rand = [rand1]() -> glm::vec2 {
    constexpr float TWO_PI = 6.28318530717958647692528676655900576;
    float theta = rand1() * TWO_PI;
    return glm::vec2 {glm::cos(theta), glm::sin(theta)};
  };

  auto curr = glm::ivec2 {1 + rand1() * 4, 1 + rand1() * 4};
  for (int try_number = 0; try_number < 10; ++try_number) 
  {
    float tree_size = rand1() * 3;
    trees.emplace_back(Tree_{
      chunk_index * glm::ivec2(CHUNK_SIZE) + curr, 
      tree_size
    });

    curr += glm::floor(glm::vec2(tree_size + 3) * circle_rand());
  }

  // secound pass tree planting
  auto plant_tree = [&](glm::ivec2 pos, float size) {
    // find the block to plant upon
    float max_height = CHUNK_HEIGHT - 1;

    // if we have not yet generated/instantiated the chunk
    if (not world.hasChunk(World::toChunk({pos.x, 0, pos.y}))) {
      return;
    }
    
    for (; max_height >= 40; --max_height) {
      if (world(pos.x, max_height, pos.y)) {
        break;
      }
    }
    if (world(pos.x, max_height, pos.y) != Terrain::GRASS) {
      return;
    }
    
    ++max_height;
    // leaf it up
    // FIXME: slow pow
    const auto tree_height = glm::pow(size * 2, 1.2);
    int floof = size * 0.75f;

    for (int i = -floof; i <= floof; ++i) 
    for (int j = -floof; j <= floof; ++j) 
    for (int k = -floof; k <= floof; ++k)
    {
      int y = max_height + tree_height + j;
      if (y < CHUNK_HEIGHT) {
        world.forceGet(pos.x + i, y, pos.y + k) = Terrain::WATER;
      }
    }

    for (int dj = 0; dj < tree_height; ++dj) {  
      int y = max_height + dj;
      if (y < CHUNK_HEIGHT) {
        world(pos.x, y, pos.y) = Terrain::DIRT;
      }
    }
  };

  for (auto tree : trees) {
    plant_tree(tree.pos, tree.size);
  }

  world._chunks[chunk_index].generated = true;
}
