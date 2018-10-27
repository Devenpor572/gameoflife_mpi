#pragma once
#include "Utils.hpp"

#include <string>

namespace pbm_writer
{
  void _convertRowToBitstream(const GolRow_t& row,
                              std::vector<uint8_t>& rBitstream);

  void _convertBoardToBitstreams(
    const GameOfLifeState& state,
    std::vector<std::vector<uint8_t>>& rBitstreams);

  void writePBM(const std::string& outputDir,
                const unsigned int maxGeneration,
                const GameOfLifeState& state);
}
