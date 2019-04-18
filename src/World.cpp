#include "World.h"
#include "Player.h"

World::World(Player& player) : _player_chunk_index(toChunk(player.blockPosition())) {
   _active_set.clear();
  for (int i = -2; i <= 2; ++i)
  for (int k = -2; k <= 2; ++k) {
    glm::ivec2 curr_index = _player_chunk_index + glm::ivec2(i, k);
    _active_set.emplace(curr_index);
  }
}


void World::handleTick(Player& player) {
  auto chunk_index = toChunk(player.blockPosition());
  
  if (_player_chunk_index != chunk_index) {
    _dirty = true;
    _player_chunk_index = chunk_index;

    _active_set.clear();
    for (int i = -2; i <= 2; ++i)
    for (int k = -2; k <= 2; ++k) {
      glm::ivec2 curr_index = chunk_index + glm::ivec2(i, k);
      _active_set.emplace(curr_index);
    }
  }
}