#include "World.h"
#include "Player.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

World::World(Player& player) : _player_chunk_index(toChunk(player.blockPosition())) {
   _active_set.clear();
  for (int i = -RENDER_DISTANCE; i <= RENDER_DISTANCE; ++i)
  for (int k = -RENDER_DISTANCE; k <= RENDER_DISTANCE; ++k) {
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
    for (int i = -RENDER_DISTANCE; i <= RENDER_DISTANCE; ++i)
    for (int k = -RENDER_DISTANCE; k <= RENDER_DISTANCE; ++k) {
      glm::ivec2 curr_index = chunk_index + glm::ivec2(i, k);
      _active_set.emplace(curr_index);
    }
  }
}