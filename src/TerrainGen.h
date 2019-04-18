#pragma once
#include <glm/glm.hpp>

struct Player;
struct World;

namespace TerrainGen {
  void spawn(World& world, Player& player, int width, int height);
  void chunk(World& world, glm::ivec2 chunk_index);
}