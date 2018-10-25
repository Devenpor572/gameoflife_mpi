#pragma once

#include <array>
#include <memory>
#include <vector>

typedef std::vector<bool> GolRow_t;
/** std::vector<std::vector<bool>> **/
typedef std::vector<GolRow_t> GolBoard_t;

struct GameOfLifeRules
{
  unsigned int maxGeneration;
  // 8 neighbors + null case
  std::array<bool, 9> birth{
    {false, false, false, true, false, false, false, false, false}};
  std::array<bool, 9> survive{
    {false, false, true, true, false, false, false, false, false}};
};

struct GameOfLifeState
{
  unsigned int id;
  unsigned int x;
  unsigned int y;
  GolBoard_t board;

  void initialize(unsigned int x, unsigned int y);
  void _initializeBoard();
};

struct GameOfLife
{
  std::string name;
  std::string author;
  std::vector<std::string> comments;
  std::string cornerString;
  std::string rulestring;
  GameOfLifeRules rules;
  GameOfLifeState state;
};

namespace utils
{
  // Read an entire file into a single std::string
  std::string readFile(std::string filename);
}
