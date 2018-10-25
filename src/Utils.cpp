#include "Utils.hpp"

#include <fstream>

void GameOfLifeState::initialize(unsigned int x, unsigned int y)
{
  this->x = x;
  this->y = y;
  _initializeBoard();
}

void GameOfLifeState::_initializeBoard()
{
  board.resize(0);
  board.reserve(y);
  for (size_t i = 0; i < y; ++i)
  {
    board.push_back(GolRow_t(x, false));
  }
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
