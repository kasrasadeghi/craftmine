#pragma once
#include <glm/vec2.hpp>

struct Player;
struct World;
struct Chunk;

namespace TerrainGen {
  void spawn(World& world, Player& player);
  void chunk(World& world, glm::ivec2 chunk_index);
  void ground(Chunk*, glm::ivec2 chunk_index);
  void caves(World& world, glm::ivec2 chunk_index);
  void trees(World& world, glm::ivec2 chunk_index);
}