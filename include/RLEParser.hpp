#pragma once

#include "Utils.hpp"
#include <string>

namespace parser
{
  void _parseComment(std::string commentString, GameOfLife& game);

  void _parseRuleString(std::string ruleString, GameOfLifeRules& rRules);

  void _parseHeader(std::string header, GameOfLifeParameters& rParameters);

  void _parseBoard(std::string boardStr,
                   GameOfLifeState& state,
                   std::string& rLastBoardLine,
                   int& currentRow,
                   bool last = false);

  bool _placeStateOnBoard(const GameOfLifeParameters& referenceParameters,
                          const GameOfLifeState& referenceState,
                          GameOfLifeParameters& rTargetParameters,
                          GameOfLifeState& rTargetState);

  bool parseRLE(std::string filename, GameOfLife& rGame);
}
