#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace Terrain {
  enum TerrainEnum : int {
    AIR = 0, GRASS = 1, STONE = 2, 
    WATER = 3, DIRT = 4, OAK = 5, 
    OAK_LEAF = 6, SNOW = 7, ICE = 8,
    SAND = 9, SAND_STONE = 10 
  };

  void setColors(std::vector<glm::vec4>& base_colors, std::vector<glm::vec4>& off_colors);

  std::string _str(TerrainEnum t);

  std::string str(u_char byte);
}