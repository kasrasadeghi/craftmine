#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>
// FIXME: is every chunk actually only loaded once?

// maps from chunks indices to chunks that need to generate towards that chunk
// to complete this pass for a chunk:
//   see if there is cave_to_be_carved for this chunk
//   if there is, iterate through all of the caves that need to be placed here
//   for each chunk in the value that was mapped to
//     generate the carve set
//     get all the elements of the carve set in this chunk
//     carve them
std::unordered_map<glm::ivec2, std::unordered_set<glm::ivec2>> cave_to_be_carved;
