#pragma once

#include <array>
#include <memory>
#include <vector>

struct GameOfLifeRules
{
  // 8 neighbors + null case
  std::array<bool, 9> birth{
    {false, false, false, true, false, false, false, false, false}};
  std::array<bool, 9> survive{
    {false, false, true, true, false, false, false, false, false}};
};

struct GameOfLifeParameters
{
  unsigned int x;
  unsigned int y;
  GameOfLifeRules rules;
};

struct GameOfLifeState
{
  explicit GameOfLifeState(unsigned int id,
                           const GameOfLifeParameters& parameters);
  unsigned int id;
  std::vector<std::vector<bool>> board;
};

struct GameOfLife
{
  std::string name;
  std::string author;
  std::vector<std::string> comments;
  std::string cornerString;
  std::string rulestring;
  unsigned int maxGeneration;
  GameOfLifeParameters parameters;
  std::shared_ptr<GameOfLifeState> pState;
};

namespace utils
{
  // Read an entire file into a single std::string
  std::string readFile(std::string filename);
}
