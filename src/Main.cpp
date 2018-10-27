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
  for (size_t j = 0; j < state.dims.y; ++j)
  {
    for (size_t i = 0; i < state.dims.x; ++i)
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
  for (size_t j = 0; j < state.dims.y; ++j)
  {
    for (size_t i = 0; i < state.dims.x; ++i)
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

bool partition(const unsigned int partitionNum,
               const unsigned int partitionSize,
               const GameOfLifeState& state,
               PartitionState& partitionState)
{
  partitionState.id = partitionNum;
  unsigned int lowerBound = partitionNum * partitionSize;
  unsigned int upperBound = (partitionNum + 1) * partitionSize;
  unsigned int y;
  if (partitionNum > 0 && upperBound < state.dims.y)
  {
    y = partitionSize;
    partitionState.hasLowerBuffer = true;
    partitionState.hasUpperBuffer = true;
  }
  else if (partitionNum == 0)
  {
    y = partitionSize;
    partitionState.hasLowerBuffer = false;
    partitionState.hasUpperBuffer = true;
  }
  else if (lowerBound < state.dims.y && upperBound >= state.dims.y)
  {
    upperBound = state.dims.y;
    y = state.dims.y - lowerBound;
    partitionState.hasLowerBuffer = true;
    partitionState.hasUpperBuffer = false;
  }
  else
  {
    return false;
  }
  partitionState.initialize(BoardDimensions(state.dims.x, y));
  if (partitionState.hasLowerBuffer)
  {
    std::copy(state.board[lowerBound - 1].begin(),
              state.board[lowerBound - 1].end(),
              partitionState.lowerBuffer.begin());
  }
  if (partitionState.hasUpperBuffer)
  {
    std::copy(state.board[upperBound].begin(),
              state.board[upperBound].end(),
              partitionState.upperBuffer.begin());
  }
  for (size_t j = 0; j < partitionState.board.size(); ++j)
  {
    std::copy(state.board[lowerBound + j].begin(),
              state.board[lowerBound + j].end(),
              partitionState.board[j].begin());
  }
  return true;
}

void computeGeneration(const GameOfLifeRules& rules, GameOfLifeState& state)
{
  std::vector<std::vector<uint8_t>> neighborCounts;
  // We want a buffer row on each side
  neighborCounts.reserve(state.dims.y + 2);
  for (size_t i = 0; i < state.dims.y + 2; ++i)
  {
    neighborCounts.push_back(std::vector<uint8_t>(state.dims.x + 2, 0));
  }
  incrementNeighbors(state, neighborCounts);
  populateNextState(rules, neighborCounts, state);
  state.id++;
}

void computePartitionGeneration(const GameOfLifeRules& rules,
                                const PartitionState& partitionState,
                                PartitionState& rState)
{
  const unsigned int lowerBufferInt = (partitionState.hasLowerBuffer ? 1 : 0);
  const unsigned int upperBufferInt = (partitionState.hasUpperBuffer ? 1 : 0);
  GameOfLifeState state;
  state.id = partitionState.id;
  state.initialize(
    BoardDimensions(partitionState.dims.x,
                    partitionState.dims.y + lowerBufferInt + upperBufferInt));
  if (partitionState.hasLowerBuffer)
  {
    std::copy(partitionState.lowerBuffer.begin(),
              partitionState.lowerBuffer.end(),
              state.board.front().begin());
  }
  for (size_t i = 0; i < partitionState.dims.y; ++i)
  {
    std::copy(partitionState.board[i].begin(),
              partitionState.board[i].end(),
              state.board[i + lowerBufferInt].begin());
  }
  if (partitionState.hasUpperBuffer)
  {
    std::copy(partitionState.upperBuffer.begin(),
              partitionState.upperBuffer.end(),
              state.board.back().begin());
  }
  computeGeneration(rules, state);
  rState.id = state.id;
  rState.initialize(partitionState.dims);
  for (size_t i = 0; i < rState.dims.y; ++i)
  {
    std::copy(state.board[i + lowerBufferInt].begin(),
              state.board[i + lowerBufferInt].end(),
              rState.board[i].begin());
  }
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
  unsigned int maxGeneration;
  if (rank == 0)
  {
    std::string inputFile = argv[1];
    outputDir = argv[2];
    maxGeneration = 1000;
    BoardDimensions dims(1024, 1024);
    partitionSize = 16;
    if (argc == 7)
    {
      maxGeneration = std::stoul(argv[3]);
      dims.x = std::stoul(argv[4]);
      dims.y = std::stoul(argv[5]);
      partitionSize = std::stoul(argv[6]);
    }
    if (!parser::parseRLE(inputFile, dims, game))
    {
      std::cerr << "Failed to parse RLE file!" << std::endl;
      return EXIT_FAILURE;
    }
  }
  // TODO Swap parameters
  if (rank == 0)
  {
    pbm_writer::writePBM(outputDir, maxGeneration, game.state);
    for (unsigned int i = 1; i <= maxGeneration; ++i)
    {
      bool partitioning = true;
      unsigned int partitionNum = 0;
      while (partitioning)
      {
        PartitionState partitionState;
        partitioning =
          partition(partitionNum, partitionSize, game.state, partitionState);

        partitionNum++;
      }
      computeGeneration(game.rules, game.state);
      pbm_writer::writePBM(outputDir, maxGeneration, game.state);
    }
  }

  // MPI_Finalize();
  std::cout << "Press any key to continue..." << std::endl;
  std::cin.get();
  return EXIT_SUCCESS;
}
