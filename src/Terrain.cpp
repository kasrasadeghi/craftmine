#include "Terrain.h"

void Terrain::setColors(std::vector<glm::vec4>& base_colors, std::vector<glm::vec4>& off_colors) {
  base_colors[AIR]   = glm::vec4(0);
  base_colors[GRASS] = glm::vec4(0.2, 0.8, 0, 1);
  base_colors[STONE] = glm::vec4(0.5, 0.5, 0.5, 1);
  base_colors[WATER] = glm::vec4(0, 0, 1, 1);
  base_colors[DIRT]  = glm::vec4(0.5, 0.3, 0, 1);

  off_colors[AIR]   = glm::vec4(0);
  off_colors[GRASS] = glm::vec4(0, 0.6, 0, 1);
  off_colors[STONE] = glm::vec4(0.2, 0.2, 0.2, 1);
  off_colors[WATER] = glm::vec4(0, 0.5, 0.5, 1);
  off_colors[DIRT]  = glm::vec4(0.3, 0.1, 0, 1);
}

std::string Terrain::_str(Terrain::TerrainEnum t) {
  switch(t) {
  case AIR:   return "Air";
  case GRASS: return "Grass";
  case STONE: return "Stone";
  case WATER: return "Water";
  case DIRT:  return "Dirt";
  default:    return "UNKNOWN BLOCK";
  }
}

std::string Terrain::str(u_char byte) {
  return Terrain::_str(static_cast<TerrainEnum>(byte));
}