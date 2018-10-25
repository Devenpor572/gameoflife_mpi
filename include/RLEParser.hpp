#pragma once

#include "Utils.hpp"
#include <string>

namespace parser
{
  void _parseComment(std::string commentString, GameOfLife& game);

  void _parseRuleString(std::string ruleString, GameOfLifeRules& rRules);

  void _parseHeader(std::string header,
                    const unsigned int maxX,
                    const unsigned int maxY,
                    GameOfLifeState& rState,
                    GameOfLifeRules& rRules);

  void _parseBoard(std::string boardStr,
                   GameOfLifeState& state,
                   std::string& rLastBoardLine,
                   int& currentRow,
                   bool last = false);

  bool _placeStateOnBoard(const GameOfLifeState& referenceState,
                          GameOfLifeState& rTargetState);

  bool parseRLE(std::string filename,
                unsigned int maxGeneration,
                unsigned int x,
                unsigned int y,
                GameOfLife& rGame);
}
