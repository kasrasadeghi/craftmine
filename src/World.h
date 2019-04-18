#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "GlmHashes.h"
#include <glm/gtc/integer.hpp>

#include <sys/types.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>

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

  void build(glm::ivec2 offset, std::vector<Instance>& instances) {
    auto addCube = [&](glm::vec3 pos, const std::array<bool, 6>& airs, GLuint texture_index) {
      if (airs[0]) instances.emplace_back(pos, 0, texture_index);
      if (airs[1]) instances.emplace_back(pos, 1, texture_index);
      if (airs[2]) instances.emplace_back(pos, 2, texture_index);
      if (airs[3]) instances.emplace_back(pos, 3, texture_index);
      if (airs[4]) instances.emplace_back(pos, 4, texture_index);
      if (airs[5]) instances.emplace_back(pos, 5, texture_index);
    };

    auto isAir = [&](int i, int j, int k) {
      if (i < 0 || j < 0 || k < 0 
        || i >= CHUNK_SIZE 
        || j >= CHUNK_HEIGHT
        || k >= CHUNK_SIZE) {
        return true;
      }

      return data[i][j][k] == 0;
    };

    for (int i = 0; i < CHUNK_SIZE; ++i) {
    for (int j = 0; j < CHUNK_HEIGHT; ++j) {
    for (int k = 0; k < CHUNK_SIZE; ++k) {
      if (data[i][j][k] != 0) {
        std::array<bool, 6> airs = {
          isAir(i+1, j, k),
          isAir(i, j+1, k),
          isAir(i, j, k+1),
          isAir(i-1, j, k),
          isAir(i, j-1, k),
          isAir(i, j, k-1),
        };

        addCube({i + offset.x, j, k + offset.y}, airs, 0);
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
      _chunks[chunk_index].build({chunk_index.x*CHUNK_SIZE, chunk_index.y*CHUNK_SIZE}, instances);
    }
  }

  bool isAir(int i, int j, int k) const {
    if (j < 0 || j >= CHUNK_HEIGHT) {
      if (hasChunk(toChunk(glm::ivec3(i, 0, k)))) {
        // loaded chunk
        return operator()(i, j, k) == 0;
      } else {
        // unloaded chunk
        return true;
      }
    }
    // wrong y
    return true;
  }

  static glm::ivec2 toChunk(glm::ivec3 block) {
    return glm::ivec2(block.x / CHUNK_SIZE, block.z / CHUNK_SIZE);
  }

  bool hasChunk(glm::ivec2 chunk_index) const {
    return _chunks.count(chunk_index) != 0;
  }

  bool isChunkLoaded(glm::ivec2 chunk_index) const {
    return _chunks.at(chunk_index).generated;
  }
};