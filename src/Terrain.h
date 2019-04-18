#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace Terrain {
  enum Terrain : int {
    AIR = 0, GRASS = 1, STONE = 2, WATER = 3, DIRT
  };

  void setColors(std::vector<glm::vec4>& base_colors, std::vector<glm::vec4>& off_colors);
}
