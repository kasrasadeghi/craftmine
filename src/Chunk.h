#include "Config.h"
#include "Terrain.h"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <array>
#include <functional>
#include <vector>
#include <cassert>

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

struct Chunk {
  std::array<std::array<std::array<u_char, CHUNK_SIZE>, CHUNK_HEIGHT>, CHUNK_SIZE> data {}; // zero init in cpp
  
  enum class State {                                    /* Generated_Trees == Generated */
    Exists = 0, Generated_Ground = 1, Generated_Caves = 2, Generated_Trees = 3, Generated = 3, Built = 4
  };
  State _state = State::Exists;

  std::vector<Instance> _instances;
  std::vector<Instance> _water_instances;

  /// copy cached instances
  void load(std::vector<Instance>& instances) {
    assert (_state >= State::Built);
    assert (not _instances.empty());
    instances.reserve(instances.size() + _instances.size());
    std::copy(_instances.begin(), _instances.end(), std::back_inserter(instances));
  }

  void load_water(std::vector<Instance>& instances) {
    assert (_state >= State::Built);
    instances.reserve(instances.size() + _water_instances.size());
    std::copy(_water_instances.begin(), _water_instances.end(), std::back_inserter(instances));
  }

  /// build instances for this chunk
  void build(glm::ivec2 offset,
    std::function<bool(int,int,int)> worldIsAir,
    std::function<bool(int,int,int)> worldIsWater) {
    assert (_state >= State::Generated);
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
    _state = State::Built;
  }
};