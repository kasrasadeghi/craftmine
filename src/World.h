#pragma once

#include <glad/glad.h>
#include <vector>
#include <sys/types.h>
#include <glm/glm.hpp>

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 128;

struct Instance {
  Instance(glm::vec3 p, GLuint d): x(p.x), y(p.y), z(p.z), direction(d) {}
  float x;
  float y;
  float z;
  GLuint direction; // 0 .. 5 = x, y, z, -x, -y, -z
} __attribute__((packed));

using std::array;
struct Chunk {

  array<array<array<u_char, CHUNK_SIZE>, CHUNK_HEIGHT>, CHUNK_SIZE> data {}; // zero init in cpp

  void build(glm::ivec2 offset, std::vector<Instance>& instances) {
    auto addCube = [&](glm::vec3 pos, const std::array<bool, 6>& airs) {
      if (airs[0]) instances.emplace_back(pos, 0);
      if (airs[1]) instances.emplace_back(pos, 1);
      if (airs[2]) instances.emplace_back(pos, 2);
      if (airs[3]) instances.emplace_back(pos, 3);
      if (airs[4]) instances.emplace_back(pos, 4);
      if (airs[5]) instances.emplace_back(pos, 5);
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

        addCube({i + offset.x, j, k + offset.y}, airs);
      }
    }}}
  }
};

struct World {
  int _width;
  int _height;
  std::vector<std::vector<Chunk>> _chunks;

  World() {
    _chunks.emplace_back();
    _chunks.back().emplace_back();
  }

  World(int width, int height) {
    int cw = width / CHUNK_SIZE;
    int ch = height / CHUNK_SIZE;
    for (int i = 0; i < cw; ++i) {
      _chunks.emplace_back();    
      for (int k = 0; k < ch; ++k) {
        _chunks.back().emplace_back();
      }
    }
    _width = cw * CHUNK_SIZE;
    _height = ch * CHUNK_SIZE;
  }

  u_char& operator()(int i, int j, int k){
    int ci = i / CHUNK_SIZE;
    int ck = k / CHUNK_SIZE;
    int di = i % CHUNK_SIZE;
    int dk = k % CHUNK_SIZE;
    return _chunks.at(ci).at(ck).data.at(di).at(j).at(dk);
  }

  void build(std::vector<Instance>& instances) {
    // _chunks[0][0].build(instances);
    for (uint i = 0; i < _chunks.size(); ++i) {
      for (uint k = 0; k < _chunks[0].size(); ++k) {
        _chunks[i][k].build({i*CHUNK_SIZE, k*CHUNK_SIZE}, instances);
      }
    }
  }
};