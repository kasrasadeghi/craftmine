#pragma once
#include <glm/glm.hpp>
#include "Terrain.h"
#include <functional>

struct Player;
struct World;

struct biome_settings {
  // generation settings
  float noise_intensity = .4f; // between .4 and .7 optimally;
  unsigned int base_height = 50;
  // block settings
  unsigned char surface_block = Terrain::STONE; 
  unsigned char sub_surf_block = Terrain::STONE;
  
  // tree settings
  unsigned char tree_block = Terrain::STONE;
  unsigned char leaf_block = Terrain::STONE;

  unsigned int forest_density;
  unsigned int average_forest_height = 3;

  // std::function<std::array<bool, 128> ()> generate_slice;
  std::string name = "DEFAULT";

  biome_settings() = default;
  biome_settings(float ni, unsigned int bh, unsigned char sb,unsigned char ssb,
    unsigned char tb, unsigned char lb, unsigned int fd, unsigned int afh, 
    std::string n) :
      noise_intensity(ni), base_height(bh), surface_block(sb), sub_surf_block(ssb), tree_block(tb),
      leaf_block(lb), forest_density(fd), average_forest_height(afh), name(n) {}
};

namespace TerrainGen {
  void spawn(World& world, Player& player);
  void chunk(World& world, glm::ivec2 chunk_index);
}