#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "GlmHashes.h"
#include <glm/gtc/integer.hpp>
#include "Terrain.h"

#include <sys/types.h> // FIXME: remove and just use GL___ types
#include <vector>
#include <unordered_map>
#include <future>
#include <algorithm>

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 128;
constexpr int GEN_DISTANCE = 10;
constexpr int RENDER_DISTANCE = 7;

struct Instance {
  Instance(glm::vec3 p, GLuint d, GLuint ti):
      x(p.x), y(p.y), z(p.z), direction(d), texture_index(ti) {}
  float x;
  float y;
  float z;

  //FIXME: use GLuchar for both direction and texture_index
  GLuint direction; // 0 .. 5 = x, y, z, -x, -y, -z
  GLuint texture_index;
} __attribute__((packed));

using std::array;
struct Chunk {
  bool generated = false;
  bool built = false;
  array<array<array<u_char, CHUNK_SIZE>, CHUNK_HEIGHT>, CHUNK_SIZE> data {}; // zero init in cpp
  std::vector<Instance> _instances;
  std::vector<Instance> _water_instances;

  /// copy cached instances
  void load(std::vector<Instance>& instances) {
    assert (generated);
    assert (built);
    assert (not _instances.empty());
    instances.reserve(instances.size() + _instances.size());
    std::copy(_instances.begin(), _instances.end(), std::back_inserter(instances));
  }

  void load_water(std::vector<Instance>& instances) {
    assert (generated);
    assert (built);
    instances.reserve(instances.size() + _water_instances.size());
    std::copy(_water_instances.begin(), _water_instances.end(), std::back_inserter(instances));
  }

  /// build instances for this chunk
  void build(glm::ivec2 offset,
    std::function<bool(int,int,int)> worldIsAir,
    std::function<bool(int,int,int)> worldIsWater) {
    assert (generated);
    _instances.clear();
    _water_instances.clear();
    auto isAir = [&](int i, int j, int k) -> bool {
      if (i > 15 || j > 15 || k > 15 || i < 0 || j < 0 || k < 0) {
        return worldIsAir(offset.x + i, j, offset.y + k);
      }
      return data[i][j][k] == 0;
    };

    auto isWater = [&](int i, int j, int k) -> bool {
      if (i > 15 || j > 15 || k > 15 || i < 0 || j < 0 || k < 0) {
        return worldIsWater(offset.x + i, j, offset.y + k);
      }
      return data[i][j][k] == Terrain::WATER;
    };

    auto addCube = [&](glm::vec3 pos, const std::array<bool, 6>& transparences, 
      GLuint texture_index, std::vector<Instance>& buff) {

      for (int i = 0; i < 6; ++i) {
        if (transparences[i]) {
          buff.emplace_back(pos, i, texture_index);
        }
      }
    };

    for (int i = 0; i < CHUNK_SIZE; ++i)
    for (int j = 0; j < CHUNK_HEIGHT; ++j)
    for (int k = 0; k < CHUNK_SIZE; ++k) 
    {
      const unsigned char& block = data[i][j][k];
      if (block != 0 && block != Terrain::WATER) {
        std::array<bool, 6> airs = {
          isAir(i+1, j,   k)  ,
          isAir(i,   j+1, k)  ,
          isAir(i,   j,   k+1),
          isAir(i-1, j,   k)  ,
          isAir(i,   j-1, k)  ,
          isAir(i,   j,   k-1),
        };
        std::array<bool, 6> waters = {
          isWater(i+1, j,   k)  ,
          isWater(i,   j+1, k)  ,
          isWater(i,   j,   k+1),
          isWater(i-1, j,   k)  ,
          isWater(i,   j-1, k)  ,
          isWater(i,   j,   k-1),
        };

        addCube({i + offset.x, j, k + offset.y}, airs, data[i][j][k], _instances);
        addCube({i + offset.x, j, k + offset.y}, waters, data[i][j][k], _instances);
      } else if (block == Terrain::WATER) {
        std::array<bool, 6> airs = {
          isAir(i+1, j,   k)  ,
          isAir(i,   j+1, k)  ,
          isAir(i,   j,   k+1),
          isAir(i-1, j,   k)  ,
          isAir(i,   j-1, k)  ,
          isAir(i,   j,   k-1),
        };

        addCube({i + offset.x, j, k + offset.y}, airs, data[i][j][k], _water_instances);
      }
    }

    assert (not _instances.empty());
    built = true;
  }
};

struct Player;

struct World {
  std::unordered_map<glm::ivec2, Chunk> _chunks;
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
    return _chunks.at(chunk_index).data.at(di).at(j).at(dk);
  }

  u_char& forceGet(int i, int j, int k) {
    auto good_mod = [](int x, int y) { return (y + (x%y)) % y; };

    int di = good_mod(i, CHUNK_SIZE);
    int dk = good_mod(k, CHUNK_SIZE);
    Chunk& chunk = _chunks[toChunk({i, j, k})];
    return chunk.data.at(di).at(j).at(dk);
  }

  u_char operator()(int i, int j, int k) const {
    auto good_mod = [](int x, int y) { return (y + (x%y)) % y; };

    int di = good_mod(i, CHUNK_SIZE);
    int dk = good_mod(k, CHUNK_SIZE);
    auto chunk_index = toChunk({i, j, k});
    assert (hasChunk(chunk_index));
    return _chunks.at(chunk_index).data.at(di).at(j).at(dk);
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
    return _chunks.at(chunk_index).generated;
  }
};
