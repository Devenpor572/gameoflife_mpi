#pragma once

#include <array>
#include <memory>
#include <vector>

typedef std::vector<bool> GolRow_t;
/** std::vector<std::vector<bool>> **/
typedef std::vector<GolRow_t> GolBoard_t;

struct GameOfLifeRules
{
  // 8 neighbors + null case
  std::array<bool, 9> birth{
    {false, false, false, true, false, false, false, false, false}};
  std::array<bool, 9> survive{
    {false, false, true, true, false, false, false, false, false}};
};

struct BoardDimensions
{
  BoardDimensions() {}
  BoardDimensions(unsigned int x, unsigned int y, unsigned int y0 = 0)
    : x(x), y(y)
  {
  }
  unsigned int x;
  unsigned int y;
};

struct GameOfLifeState
{
  unsigned int id;
  BoardDimensions dims;
  GolBoard_t board;

  virtual void initialize(const BoardDimensions& dims);
  void _initializeBoard();
};

struct PartitionState : GameOfLifeState
{
  bool hasLowerBuffer;
  bool hasUpperBuffer;
  GolRow_t lowerBuffer;
  GolRow_t upperBuffer;
  virtual void initialize(const BoardDimensions& dims);
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
