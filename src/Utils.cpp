#include "Utils.hpp"

#include <fstream>

GameOfLifeState::GameOfLifeState(unsigned int id,
                                 const GameOfLifeParameters& parameters)
  : id(id)
{
  board.resize(0);
  board.reserve(parameters.y);
  for (size_t i = 0; i < parameters.y; ++i)
  {
    board.push_back(std::vector<bool>(parameters.x, false));
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
