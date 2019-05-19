#pragma once

#include <unordered_set>

#include <glm/gtx/hash.hpp>
#include <glm/vec2.hpp>

struct Player;
struct World;
struct Chunk;

namespace TerrainGen {
  void spawn(World& world, Player& player);
  void chunk(World& world, glm::ivec2 chunk_index);
  
  void ground(Chunk*, glm::ivec2 chunk_index);

  std::unordered_set<glm::ivec3> carve_set(glm::ivec2 chunk_index);
  void caves(World& world, glm::ivec2 chunk_index);
  void trees(World& world, glm::ivec2 chunk_index);
}