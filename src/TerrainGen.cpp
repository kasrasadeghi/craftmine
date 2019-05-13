#include "TerrainGen.h"
#include "World.h"
#include "Player.h"
#include "Perlin.h"
#include "Terrain.h"

#include <glm/gtx/hash.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <unordered_set>

// FIXME: is every chunk actually only loaded once?

std::unordered_set<glm::ivec3> cave_voxels_to_be_carved;

void TerrainGen::spawn(World& world, Player& player) {
  auto chunk_index = World::toChunk(player.blockPosition());

  for (int i = -RENDER_DISTANCE; i <= RENDER_DISTANCE; ++i) {
    for (int k = -RENDER_DISTANCE; k <= RENDER_DISTANCE; ++k) {
      glm::ivec2 curr_index = chunk_index + glm::ivec2(i, k);
      world._chunks.emplace(curr_index, new Chunk());
      chunk(world, curr_index);
    }
  }
}

void TerrainGen::chunk(World& world, glm::ivec2 chunk_index) {
  assert (world.hasChunk(chunk_index));
  assert (not world._chunks.at(chunk_index)->generated);

  int bi = chunk_index.x * CHUNK_SIZE;
  int bk = chunk_index.y * CHUNK_SIZE;

  /// Base generation pass

  auto stretch_octave = [](int s) {
    using namespace Terrain;
    if (s < 2) return GRASS;
    if (s < 8) return DIRT;
    return STONE;
  };

  // FIXME: check: is everything in this function actually in the same chunk?

  for (int di = 0; di < CHUNK_SIZE; ++di)
  for (int dk = 0; dk < CHUNK_SIZE; ++dk)
  {
    int i = bi + di;
    int k = bk + dk;

    // solidity of block
    bool column[128] = {}; // all elements zero

    for (int y = 0; y < 128; ++y) {
      if (not column[y]) {

        // float gradient =  1 + 1/p2 - y/64.f;
        float p2 = perlin(i / 150.f, 0, k / 150.f);
        float p = perlin(i / 150.f, y / 128.f, k / 150.f);

        float scalefac = .4f + .4f * p2;
        float gradient = (2 + p2) - y/64.f;

        column[y] = glm::floor(glm::mix(gradient, p, scalefac));
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

  /// rand wrappers

  auto rand1 = []() -> float {
    return rand()/(float)RAND_MAX;
  };

  auto circle_rand = [rand1]() -> glm::vec2 {
    constexpr float TWO_PI = 6.28318530717958647692528676655900576;
    float theta = rand1() * TWO_PI;
    return glm::vec2 {glm::cos(theta), glm::sin(theta)};
  };

  /// Cave generation pass

  // randomly pick a point in the chunk
  if (perlin(bi/15.f, bk/15.f) < 0.2) {

    auto point = glm::vec3(bi + perlin(bi/15.f, bk/15.f) * 15, perlin(bk/15.f, bi/15.f) * 128, bk + perlin(bi/15.f, bk/15.f, (bi ^ bk)/15.f) * 15);

    // map some perlin segments
    constexpr int point_count = 20;
    std::array<glm::vec3, point_count> cave_points {};
    cave_points[0] = point;
    for (int i = 1; i < point_count; ++i) {
      auto prev = cave_points[i - 1];

      float theta = glm::two_pi<float>() * perlin(prev.x / 15.f, prev.y / 15.f, prev.z / 15.f);
      float phi   = glm::pi<float>() * perlin(prev.x / 15.f, prev.y / 15.f, prev.z / 15.f);

      auto toSpherical = [](float radius, float theta, float phi) -> glm::vec3 {
        return glm::vec3(
          radius * glm::cos(theta) * glm::sin(phi),
          radius * glm::sin(theta) * glm::sin(phi),
          radius * glm::cos(phi)
        );
      };

      cave_points[i] = prev + toSpherical(3, theta, phi);
    }

    std::unordered_set<glm::ivec3> carve_voxel_set;
    auto carve_ball = [](glm::vec3 curr_pos, std::unordered_set<glm::ivec3>& carve_set) {
      
      constexpr int sphere_radius = 5;
      for (int ri = -sphere_radius; ri < sphere_radius; ++ri)
      for (int rj = -sphere_radius; rj < sphere_radius; ++rj)
      for (int rk = -sphere_radius; rk < sphere_radius; ++rk)
      {
        glm::ivec3 current_voxel = glm::ivec3(curr_pos) + glm::ivec3(ri, rj, rk);
        if (glm::distance(glm::vec3(current_voxel), curr_pos) < sphere_radius) {
          carve_set.emplace(current_voxel);
        }
      }
    };

    // carve out cave
    for (int i = 0; i < point_count - 1; ++i) {

      // interpolate between point 0 and point 1
      for (float d = 0; d < 1; d += 0.3) {
        glm::vec3 curr_pos = glm::mix(cave_points[i], cave_points[i + 1], d);
        carve_ball(curr_pos, carve_voxel_set);
      }
    }

    for (glm::ivec3 voxel : carve_voxel_set) {
      if (voxel.y >= 0 && voxel.y < CHUNK_HEIGHT) {
        if (voxel.x >= bi && voxel.z >= bk
            && voxel.x < bi + CHUNK_SIZE && voxel.z < bk + CHUNK_SIZE) {
          auto& block = world(voxel.x, voxel.y, voxel.z);
          if (block != Terrain::WATER) {
            block = Terrain::AIR;
          }
        } else {
          cave_voxels_to_be_carved.emplace(voxel);
        }
      }
    }
  }

  world._chunks.at(chunk_index)->generated = true;

  // neighboring caves
  for (glm::ivec3 voxel : cave_voxels_to_be_carved) {
    glm::ivec2 voxel_chunk_index = World::toChunk(voxel);
    if (world.hasChunk(voxel_chunk_index) && world._chunks.at(voxel_chunk_index)->generated) {
      auto& block = world(voxel.x, voxel.y, voxel.z);
      if (block != Terrain::WATER) {
        block = Terrain::AIR;
      }
    }
  }

  /// Tree pass
  struct Tree_ {
    glm::ivec2 pos;
    float size; // 0 .. 3
  };

  // decide where to put trees
  std::vector<Tree_> trees;

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

    for (int j = -floof; j <= floof; ++j) {
      int floof_layer_radius = floof - (j - floof);
      for (int i = -floof_layer_radius; i <= floof_layer_radius; ++i) 
      for (int k = -floof_layer_radius; k <= floof_layer_radius; ++k)
      {
        int y = max_height + tree_height + j;
        if (y < CHUNK_HEIGHT) {
          world.forceGet(pos.x + i, y, pos.y + k) = Terrain::LEAF;
        }
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

}
