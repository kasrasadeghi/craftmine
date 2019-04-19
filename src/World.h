#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "GlmHashes.h"
#include <glm/gtc/integer.hpp>

#include <sys/types.h> // FIXME: remove and just use GL___ types
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <future>

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 128;
constexpr int GEN_DISTANCE = 10;
constexpr int RENDER_DISTANCE = 5;

struct Instance {
  Instance(glm::vec3 p, GLuint d, GLuint ti):
      x(p.x), y(p.y), z(p.z), direction(d), texture_index(ti) {}
  float x;
  float y;
  float z;
  GLuint direction; // 0 .. 5 = x, y, z, -x, -y, -z
  GLuint texture_index;
} __attribute__((packed));

using std::array;
struct Chunk {
  bool generated = false;
  array<array<array<u_char, CHUNK_SIZE>, CHUNK_HEIGHT>, CHUNK_SIZE> data {}; // zero init in cpp
  std::vector<Instance> _instances;

  void build(glm::ivec2 offset, std::vector<Instance>& instances, std::function<bool(int,int,int)> isAir) {
    if (not _instances.empty()) {
      instances.reserve(instances.size() + _instances.size());
      std::copy(_instances.begin(), _instances.end(), std::back_inserter(instances));
      return;
    }

    auto addCube = [&](glm::vec3 pos, const std::array<bool, 6>& airs, GLuint texture_index) {
      for (int i = 0; i < 6; ++i) {
        if (airs[i]) {
          instances.emplace_back(pos, i, texture_index);
          _instances.emplace_back(pos, i, texture_index);
        }
      }
    };

    for (int i = 0; i < CHUNK_SIZE; ++i) {
    for (int j = 0; j < CHUNK_HEIGHT; ++j) {
    for (int k = 0; k < CHUNK_SIZE; ++k) {
      if (data[i][j][k] != 0) {
        std::array<bool, 6> airs = {
          isAir(offset.x + i+1, j,   offset.y + k),
          isAir(offset.x + i,   j+1, offset.y + k),
          isAir(offset.x + i,   j,   offset.y + k+1),
          isAir(offset.x + i-1, j,   offset.y + k),
          isAir(offset.x + i,   j-1, offset.y + k),
          isAir(offset.x + i,   j,   offset.y + k-1),
        };

        addCube({i + offset.x, j, k + offset.y}, airs, data[i][j][k]);
      }
    }}}
  }
};

struct Player;

struct World {
  std::unordered_map<glm::ivec2, Chunk> _chunks;
  std::unordered_set<glm::ivec2> _active_set;
  glm::ivec2 _player_chunk_index;
  bool _dirty = true;

  World(Player& player);

  bool dirty() const { return _dirty; }

  void handleTick(Player& player);

  u_char& operator()(int i, int j, int k) {
    auto good_mod = [](int x, int y) { return (y + (x%y)) % y; };

    int ci = i / CHUNK_SIZE;
    int ck = k / CHUNK_SIZE;
    int di = good_mod(i, CHUNK_SIZE);
    int dk = good_mod(k, CHUNK_SIZE);
    // sometimes default constructs
    return _chunks[glm::ivec2{ci, ck}].data.at(di).at(j).at(dk);
  }

  u_char operator()(int i, int j, int k) const {
    auto good_mod = [](int x, int y) { return (y + (x%y)) % y; };

    int ci = i / CHUNK_SIZE;
    int ck = k / CHUNK_SIZE;
    int di = good_mod(i, CHUNK_SIZE);
    int dk = good_mod(k, CHUNK_SIZE);
    // sometimes default constructs
    return _chunks.at(glm::ivec2{ci, ck}).data.at(di).at(j).at(dk);
  }

  void build(std::vector<Instance>& instances) {
    instances.clear();
    for (const glm::ivec2& chunk_index : _active_set) {
      _chunks[chunk_index].build({chunk_index.x*CHUNK_SIZE, chunk_index.y*CHUNK_SIZE}, instances, [&](int i, int j, int k){return isAir(i, j, k);});
    }
  }

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

  static glm::ivec3 toBlock(glm::vec3 pos) {
    return glm::round(pos);
  }

  static glm::ivec2 toChunk(glm::ivec3 block) {
    return glm::ivec2(block.x / CHUNK_SIZE, block.z / CHUNK_SIZE);
  }

  bool hasChunk(glm::ivec2 chunk_index) const {
    return _chunks.count(chunk_index) != 0;
  }

  bool isChunkGenerated(glm::ivec2 chunk_index) const {
    return _chunks.at(chunk_index).generated;
  }
};