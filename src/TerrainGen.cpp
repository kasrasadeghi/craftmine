#include "TerrainGen.h"
#include "World.h"
#include "Player.h"
#include "Perlin.h"
#include "Terrain.h"

#include <iostream>
#include <glm/gtx/string_cast.hpp>

constexpr float BIOME_SIZE = 300.0f;

// FIXME: is every chunk actually only loaded once?
std::vector<biome_settings> biomes {
  biome_settings(0.6f, 65,Terrain::SNOW, Terrain::ICE, Terrain::OAK, Terrain::OAK_LEAF,
    3, 3, "icy"), 
  biome_settings(0.1f, 55,Terrain::GRASS, Terrain::DIRT, Terrain::OAK, Terrain::OAK_LEAF,
    3, 3, "forest"),
  biome_settings(0.01f, 45,Terrain::SAND, Terrain::SAND_STONE, Terrain::GRASS, Terrain::AIR,
    3, 3, "desert"),
    biome_settings(0.1f, 45,Terrain::GRASS, Terrain::DIRT, Terrain::OAK, Terrain::OAK_LEAF,
    3, 3, "forest2")
};



int highestBlock(World& w, int x, int z) {
  for(int j = CHUNK_HEIGHT-1; j >= 0; --j) {
    if (w(x, j, z)) return j;
  }
  return -1;
}

void cutRiver(World &w, int i, int k) {
  float water_factor = 0;
  float river_intesity = 1.f;
  constexpr int local_max_radius = 12;

  const int start_j = highestBlock(w, i, k);

  bool strike = false;
  // Check block around at a small radius

  auto check_dir = [&](glm::vec2 mod) -> float {
    int max_height_diff = 0;
    int max_radius_diff = 0; 

    int b_j = 0;
    int height_diff = 0;

    float local_water_factor = 0;
    for (int radius = 0; radius < local_max_radius; ++radius) {    
      b_j = highestBlock(w, i + mod.x * radius, k + mod.y * radius);
      if (b_j == -1) b_j = 40;
      height_diff = b_j - start_j;
      
      if (w(i + mod.x * radius, b_j, k + mod.y * radius) == Terrain::WATER) ++local_water_factor;
      
      if (height_diff < max_height_diff) {
        if (not strike) strike = true;
        else break;
        break;
      } else {
        max_height_diff = height_diff;
      }
      max_radius_diff = radius;
      local_water_factor /= local_max_radius;
      water_factor += local_water_factor;
    }  
    return 1.f * max_height_diff /(CHUNK_HEIGHT - 40) + 1.f * max_radius_diff/ local_max_radius + water_factor;
  };

  float river_score = check_dir({1, 0}) + check_dir({0, 1}) + check_dir({-1, 0}) + check_dir({0, -1});
  river_score /= 4;
    // std::cout << river_score << std::endl;
  if (river_score >= .75 && start_j > 40) {
    constexpr int river_height = 3;
    for (int l = 0; l < river_height; ++l) {
    // std::cout << "planting river with score " << river_score << " : " << i << " " << start_j << " " << k << std::endl;
      w(i, start_j - l, k) = Terrain::WATER;
    }
  }
}



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
  biome_settings settings;
  auto octave = [&settings](int h, bool v) {
    using namespace Terrain;
    if (h < 50 && v) { return STONE; }
    if (h < 55 && v) { return (TerrainEnum)(int)settings.sub_surf_block; }
    if (v)           { return (TerrainEnum)(int)settings.surface_block; }
    if (h < 45 && not v) { return AIR; }
    return AIR;
  };

  auto stretch_octave = [&settings](int s) {
    using namespace Terrain;
    if (s < 2) return (TerrainEnum)(int)settings.surface_block;
    if (s < 8) return (TerrainEnum)(int)settings.sub_surf_block;
    return STONE;
  };

  // FIXME: is everything in this function actually in the same chunk?

  for (int di = 0; di < CHUNK_SIZE; ++di)
  for (int dk = 0; dk < CHUNK_SIZE; ++dk)
  {
    int i = bi + di;
    int k = bk + dk;
    
    float biome_val = perlin(i / BIOME_SIZE, 0, k / BIOME_SIZE) * biomes.size();
    int biome_floor = glm::floor(biome_val);
    int biome_ceil  = glm::ceil(biome_val);
    biome_ceil  = biome_ceil % biomes.size();
    settings = biomes[biome_floor];
    biome_settings settings2 = biomes[biome_ceil];

    // INTERPOLATE BIOME GEN DATA
    float interp_noise = biome_val - biome_floor;
    // interp_noise = interp_noise * interp_noise * interp_noise;
    interp_noise = (1- interp_noise) * settings.noise_intensity + 
      (interp_noise) * settings2.noise_intensity;

    interp_noise = glm::clamp(0.0f, interp_noise, 1.0f);

    // bool_column[128] = settings.generate_slice().data();

    // solidity of block
    bool column[128] = {}; // all elements zero

    for (int y = 0; y < 128; ++y) {
      if (not column[y]) {        
        // float gradient =  1 + 1/p2 - y/64.f;
        float p2 = perlin(i / 150.f, 0, k / 150.f);
        float p = 2 * perlin(i / 150.f, y / 128.f, k / 150.f);

        float scalefac = interp_noise;
        float gradient = (2) - y/64.f;
        // std::cout << scalefac << std::endl;
        column[y] = glm::floor(glm::mix(gradient, p, interp_noise));
        // column[y] = glm::floor(scale0);
      }
    }
    
    int stretch = 0;
    bool seen = false;

    settings = biomes[glm::floor(perlin(i / BIOME_SIZE, 0, k / BIOME_SIZE) * biomes.size())];

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
  
  // CARVE RIVERS
  // for (int c1 =  -1; c1 < 2; ++c1)
  // for (int c2 =  -1; c2 < 2; ++c2)
  // for (int di = 0; di < CHUNK_SIZE; ++di)
  // for (int dk = 0; dk < CHUNK_SIZE; ++dk)
  // {
  //   int i = bi + di + 16 * c1;
  //   int k = bk + dk + 16 * c2;
  //   cutRiver(world, i, k);
  // }


  bool activate_trees = true;

  if (activate_trees) {
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

      // FIXME: SLOW AF, extract and optimize
      // secound pass tree planting
      auto plant_tree = [&](glm::ivec2 pos, float size) {
        // find the block to plant upon
        float max_height = CHUNK_HEIGHT - 1;
        
        for (; max_height >= 40; --max_height) {
          if (world(pos.x, max_height, pos.y)) {
            break;
          }
        }
        settings = biomes[glm::floor(perlin(pos.x / BIOME_SIZE, 0, pos.y / BIOME_SIZE) * biomes.size())];
        if (world(pos.x, max_height, pos.y) != settings.surface_block) {
          return;
        }
        
        ++max_height;
        // FIXME: slow pow
        const auto tree_height = glm::pow(size * 2, 1.2);
        int floof = size * 0.75f;

        // leaf it up
        for (int i = -floof; i <= floof; ++i) 
        for (int j = -floof; j <= floof; ++j) 
        for (int k = -floof; k <= floof; ++k) 
        {
          int y = max_height + tree_height + j;
          if (y < CHUNK_HEIGHT) {
            world(pos.x + i, y, pos.y + k) = settings.leaf_block;
          }
        }

        for (int dj = 0; dj < tree_height; ++dj) {  
          int y = max_height + dj;
          if (y < CHUNK_HEIGHT) {
            world(pos.x, y, pos.y) = settings.tree_block;
          }
        }
      };

      for (auto tree : trees) {
        plant_tree(tree.pos, tree.size);
      }
  }

  world._chunks[chunk_index].generated = true;
}