#include "Utils.hpp"

#include <fstream>

void GameOfLifeState::initialize(const BoardDimensions& dims)
{
  this->dims = dims;
  _initializeBoard();
}

void GameOfLifeState::_initializeBoard()
{
  board.resize(0);
  board.reserve(dims.y);
  for (size_t i = 0; i < dims.y; ++i)
  {
    board.push_back(GolRow_t(dims.x, false));
  }
}

void PartitionState::initialize(const BoardDimensions& dims)
{
  GameOfLifeState::initialize(dims);
  lowerBuffer = GolRow_t(dims.x, false);
  upperBuffer = GolRow_t(dims.x, false);
}

std::string utils::readFile(std::string filename)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  std::string contents;
  if (in)
  {
    in.seekg(0, std::ios::end);
    contents.resize(static_cast<size_t>(in.tellg()));
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
  }
  return contents;
}
