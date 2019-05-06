#include "World.h"
#include "Player.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

World::World(Player& player) : _player_chunk_index(toChunk(player.blockPosition())) {
  updateActiveSet(player);
}

void World::handleTick(Player& player) {
  auto chunk_index = toChunk(player.blockPosition());
  
  if (_player_chunk_index != chunk_index) {
    _might_need_generation = true;
    _player_chunk_index = chunk_index;
    updateActiveSet(player);
  }
}

void World::updateActiveSet(Player& player) {
  auto chunk_index = toChunk(player.blockPosition());

  _active_set.clear();
  for (int i = -RENDER_DISTANCE; i <= RENDER_DISTANCE; ++i)
  for (int k = -RENDER_DISTANCE; k <= RENDER_DISTANCE; ++k) {
    glm::ivec2 curr_index = chunk_index + glm::ivec2(i, k);
    _active_set.emplace_back(curr_index);
  }
  glm::vec2 pos {player.head().x, player.head().z};
  std::sort(_active_set.begin(), _active_set.end(), [pos](glm::ivec2 a, glm::ivec2 b) {
    return (glm::distance(glm::vec2(a * glm::ivec2(16)), pos)) 
          < (glm::distance(glm::vec2(b * glm::ivec2(16)), pos));
  });
}

// requires that every element of _active_set be present in _chunks and be generated
void World::build(std::vector<Instance>& instances, Player& player) {

  instances.clear();

  auto build_chunk = [&](glm::ivec2 chunk_index) -> bool {
    assert (hasChunk(chunk_index));
    // assert (isChunkGenerated(chunk_index));
    return _chunks[chunk_index].build({chunk_index.x*CHUNK_SIZE, chunk_index.y*CHUNK_SIZE}, instances, [&](int i, int j, int k){return isAir(i, j, k);});
  };

  // separate _active_set into chunks that need to be built and chunks that are already built
  std::vector<glm::ivec2> to_be_built_set;
  std::copy_if(_active_set.begin(), _active_set.end(), std::back_inserter(to_be_built_set), 
    [this](glm::ivec2 chunk_index){ return _chunks.at(chunk_index)._instances.empty(); } );

  std::vector<glm::ivec2> already_built_set;
  std::copy_if(_active_set.begin(), _active_set.end(), std::back_inserter(already_built_set), 
    [this](glm::ivec2 chunk_index){ return not _chunks.at(chunk_index)._instances.empty(); } );

  // build the chunks that already have instances
  for (const auto& chunk_index : already_built_set) {
    auto b = build_chunk(chunk_index);
    assert(not b);
  }

  // build the chunks that need new instances until the limit is hit
  constexpr int new_count_limit = 1;
  bool incomplete = false;
  int new_count = 0;
  for (const auto& chunk_index : to_be_built_set) {
    if (build_chunk(chunk_index)) {
      new_count ++;
      if (new_count > new_count_limit) {
        incomplete = true;
        break;
      }
    }
  }

  // if (not incomplete) {
    // NOTE: we could have another variable that tells us whether we need to have further building to signal to the main thread
    //   to not build and waste the time clearing/copying, but there might be a more efficient way of doing this
    //   -> potentially cache { previous active sets, chunks represented in instance vector }
    // _might_need_building = false;
    // _might_need_generation = false;
  // }
}