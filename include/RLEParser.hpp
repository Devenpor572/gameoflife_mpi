#pragma once

#include "Utils.hpp"
#include <string>

namespace parser
{
  void _parseComment(std::string commentString, GameOfLife& game);

  void _parseRuleString(std::string ruleString, GameOfLifeRules& rRules);

  void _parseHeader(std::string header,
                    const BoardDimensions& maxDims,
                    GameOfLifeState& rState,
                    GameOfLifeRules& rRules);

  void _parseBoard(std::string boardStr,
                   GameOfLifeState& state,
                   std::string& rLastBoardLine,
                   int& currentRow,
                   bool last = false);

  bool _placeStateOnBoard(const GameOfLifeState& referenceState,
                          GameOfLifeState& rTargetState);

  bool parseRLE(std::string filename, BoardDimensions dims, GameOfLife& rGame);
}