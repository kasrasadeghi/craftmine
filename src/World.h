#pragma once

#include "Terrain.h"
#include "Chunk.h"

#include <glm/gtx/hash.hpp>
#include <glm/gtc/integer.hpp>

#include <sys/types.h> // FIXME: remove and just use GL___ types
#include <vector>
#include <unordered_map>
#include <algorithm>


struct Player;

struct World {
  std::unordered_map<glm::ivec2, Chunk*> _chunks;
  std::vector<glm::ivec2> _active_set; // invariant: in increasing distance from the player
  glm::ivec2 _player_chunk_index;

  World(Player& player);

  void handleTick(Player& player);

  void updateActiveSet(Player& player);

  u_char& operator()(int i, int j, int k) {
    auto good_mod = [](int x, int y) { return (y + (x%y)) % y; };

    int di = good_mod(i, CHUNK_SIZE);
    int dk = good_mod(k, CHUNK_SIZE);
    auto chunk_index = toChunk({i, j, k});
    assert (hasChunk(chunk_index));
    return _chunks.at(chunk_index)->data.at(di).at(j).at(dk);
  }

  u_char& forceGet(int i, int j, int k) {
    auto good_mod = [](int x, int y) { return (y + (x%y)) % y; };

    int di = good_mod(i, CHUNK_SIZE);
    int dk = good_mod(k, CHUNK_SIZE);
    auto chunk_index = toChunk({i, j, k});
    if (not hasChunk(chunk_index)) {
      _chunks.emplace(chunk_index, new Chunk());
    }
    return _chunks.at(chunk_index)->data.at(di).at(j).at(dk);
  }

  u_char operator()(int i, int j, int k) const {
    auto good_mod = [](int x, int y) { return (y + (x%y)) % y; };

    int di = good_mod(i, CHUNK_SIZE);
    int dk = good_mod(k, CHUNK_SIZE);
    auto chunk_index = toChunk({i, j, k});
    assert (hasChunk(chunk_index));
    return _chunks.at(chunk_index)->data.at(di).at(j).at(dk);
  }

  void build(std::vector<Instance>& instances);
  void build_water(std::vector<Instance>& instances);

  void buildChunk(glm::ivec2 chunk_index);

  bool isAir(int i, int j, int k) const {
    if (j >= 0 && j < CHUNK_HEIGHT) {
      if (hasChunk(toChunk(glm::ivec3(i, 0, k)))) {
        // loaded chunk
        auto block = operator()(i, j, k);
        return block == 0;
      } else {
        // unloaded chunk
        return true;
      }
    }
    // wrong y
    return true;
  }

  bool isWater(int i, int j, int k) const {
    if (j >= 0 && j < CHUNK_HEIGHT) {
      if (hasChunk(toChunk(glm::ivec3(i, 0, k)))) {
        // loaded chunk
        auto block = operator()(i, j, k);
        return block == Terrain::WATER;
      } else {
        // unloaded chunk
        return true;
      }
    }
    // wrong y
    return true;
  }

  static glm::ivec3 toBlock(glm::vec3 pos) {
    return glm::round(pos);
  }

  static glm::ivec2 toChunk(glm::ivec3 block) {
    return glm::ivec2(block.x / CHUNK_SIZE, block.z / CHUNK_SIZE);
  }

  bool hasChunk(glm::ivec2 chunk_index) const {
    return _chunks.count(chunk_index);
  }

  bool isChunkGenerated(glm::ivec2 chunk_index) const {
    return _chunks.at(chunk_index)->generated;
  }
};
