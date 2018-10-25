#include "PBMWriter.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

void pbm_writer::_convertRowToBitstream(const GolRow_t& row,
                                        std::vector<uint8_t>& rBitstream)
{
  auto div = static_cast<size_t>(row.size() / 8);
  auto rem = row.size() % 8 > 0 ? 1 : 0;

  // Initialize bitstream to ceil(1/8) the size of the row
  // Fill it with 0s
  rBitstream = std::vector<uint8_t>(div + rem, 0);
  unsigned int i = 0;
  for (auto cell : row)
  {
    // Set the ith bit
    if (cell)
    {
      auto byteIdx = static_cast<size_t>(i / 8);
      // TRICKY: Write from left to right, meaning largest bit to smallest bit
      auto bitIdx = 1 << ((8 - 1) - (i % 8));
      rBitstream[byteIdx] |= bitIdx;
    }
    ++i;
  }
}

void pbm_writer::_convertBoardToBitstreams(
  const GameOfLifeState& state, std::vector<std::vector<uint8_t>>& rBitstreams)
{
  for (auto&& row : state.board)
  {
    std::vector<uint8_t> bitstream;
    pbm_writer::_convertRowToBitstream(row, bitstream);
    rBitstreams.push_back(bitstream);
  }
}

void pbm_writer::writePBM(const std::string& outputDir, const GameOfLife& game)
{
  std::vector<std::vector<uint8_t>> bitstreams;
  pbm_writer::_convertBoardToBitstreams(game.state, bitstreams);
  std::stringstream filenameSS;
  filenameSS << outputDir << "/";
  filenameSS << std::setfill('0')
             << std::setw(static_cast<unsigned int>(
                  std::log10(game.rules.maxGeneration > game.state.id
                               ? game.rules.maxGeneration
                               : game.state.id) +
                  1))
             << game.state.id;
  filenameSS << ".pbm";
  std::stringstream outputSS;
  outputSS << "P4 " << game.state.x << " " << game.state.y << std::endl;
  for (auto&& bitstream : bitstreams)
  {
    outputSS.write((const char*)&bitstream[0], bitstream.size());
  }
  std::ofstream output(filenameSS.str(), std::ios::out | std::ios::binary);
  output.write(outputSS.str().c_str(), outputSS.str().size());
  output.close();
}
