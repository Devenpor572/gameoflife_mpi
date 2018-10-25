#include "PBMWriter.hpp"
#include "RLEParser.hpp"
#include "Utils.hpp"

//#include <mpi.h>

#include <cctype>
#include <cerrno>
#include <iostream>
#include <locale>

// Output includes
#include <chrono>
#include <sstream>
#include <thread>

void incrementCellNeighbors(size_t col,
                            size_t row,
                            std::vector<std::vector<uint8_t>>& neighborCounts)
{
  size_t minCol = col - 1;
  size_t maxCol = col + 1;
  size_t minRow = row - 1;
  size_t maxRow = row + 1;
  // Increment the cells around
  for (size_t i = minCol; i <= maxCol; ++i)
  {
    neighborCounts[minRow][i]++;
    neighborCounts[maxRow][i]++;
  }
  neighborCounts[row][minCol]++;
  neighborCounts[row][maxCol]++;
}

void incrementNeighbors(const GameOfLifeState& state,
                        std::vector<std::vector<uint8_t>>& neighborCounts)
{
  for (size_t j = 0; j < state.y; ++j)
  {
    for (size_t i = 0; i < state.x; ++i)
    {
      if (state.board[j][i])
      {
        // Neighbor counts has buffer rows and columns, hence the need to add 1
        incrementCellNeighbors(i + 1, j + 1, neighborCounts);
      }
    }
  }
}

void populateNextState(const GameOfLifeRules& rules,
                       const std::vector<std::vector<uint8_t>>& neighborCounts,
                       GameOfLifeState& state)
{
  for (size_t j = 0; j < state.y; ++j)
  {
    for (size_t i = 0; i < state.x; ++i)
    {
      // Cell is alive
      if (state.board[j][i])
      {
        // Does the new cell survive?
        state.board[j][i] = rules.survive[neighborCounts[j + 1][i + 1]];
      }
      // Cell is dead
      else
      {
        // Is a new cell born?
        state.board[j][i] = rules.birth[neighborCounts[j + 1][i + 1]];
      }
    }
  }
}

void computeGeneration(const GameOfLifeRules& rules, GameOfLifeState& state)
{
  std::vector<std::vector<uint8_t>> neighborCounts;
  // We want a buffer row on each side
  neighborCounts.reserve(state.y + 2);
  for (size_t i = 0; i < state.y + 2; ++i)
  {
    neighborCounts.push_back(std::vector<uint8_t>(state.x + 2, 0));
  }
  incrementNeighbors(state, neighborCounts);
  populateNextState(rules, neighborCounts, state);
  state.id++;
}

int main(int argc, char** argv)
{
  // MPI_Init(&argc, &argv);
  int rank = 0; //, size;
  // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  // MPI_Comm_size(MPI_COMM_WORLD, &size);
  if (argc != 3 && argc != 7)
  {
    std::cerr << "Invalid parameters" << std::endl;
    return EXIT_FAILURE;
  }
  std::string outputDir;
  GameOfLife game;
  unsigned int partitionSize;
  if (rank == 0)
  {
    std::string inputFile = argv[1];
    outputDir = argv[2];
    unsigned int maxGeneration = 1000;
    unsigned int x = 1024;
    unsigned int y = 1024;
    partitionSize = 16;
    if (argc == 7)
    {
      maxGeneration = std::stoul(argv[3]);
      x = std::stoul(argv[4]);
      y = std::stoul(argv[5]);
      partitionSize = std::stoul(argv[6]);
    }
    if (!parser::parseRLE(inputFile, maxGeneration, x, y, game))
    {
      std::cerr << "Failed to parse RLE file!" << std::endl;
      return EXIT_FAILURE;
    }
  }
  // TODO Swap parameters
  if (rank == 0)
  {
    pbm_writer::writePBM(outputDir, game);
    for (unsigned int i = 1; i <= game.rules.maxGeneration; ++i)
    {
      computeGeneration(game.rules, game.state);
      pbm_writer::writePBM(outputDir, game);
    }
  }

  // MPI_Finalize();
  std::cout << "Press any key to continue..." << std::endl;
  std::cin.get();
  return EXIT_SUCCESS;
}
