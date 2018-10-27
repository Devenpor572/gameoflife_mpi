#include "RLEParser.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace
{
  enum class ParseStage
  {
    NONE,
    COMMENT,
    HEADER,
    BODY,
    COMPLETE
  };
}

void parser::_parseComment(std::string commentString, GameOfLife& game)
{
  char type = commentString.at(1);
  commentString = commentString.substr(3);
  switch (type)
  {
  case 'c': // Fall through
  case 'C': // Ignore
    game.comments.push_back(commentString);
    break;
  case 'N':
    game.name = commentString;
    break;
  case 'O':
    game.author = commentString;
    break;
  case 'P': // Represents top left corner, fall through
  case 'R': // Also top left corner
    game.cornerString = commentString;
    break;
  case 'r': // Rulestring
    game.rulestring = commentString;
    break;
  }
}

void parser::_parseRuleString(std::string ruleString, GameOfLifeRules& rRules)
{
  const std::regex BS_RE(R"(B\d+/S\d+)");
  const std::regex SB_RE(R"(\d+/\d+)");
  GameOfLifeRules rules;
  std::istringstream ruleStringSS(ruleString);
  bool firstParse = true;
  for (std::string ruleStringElement;
       std::getline(ruleStringSS, ruleStringElement, '/');)
  {
    if (std::regex_match(ruleString, BS_RE))
    {
      char type = ruleStringElement.at(0);
      ruleStringElement = ruleStringElement.substr(1);
      switch (type)
      {
      case 'B':
        for (char ch : ruleStringElement)
        {
          rules.birth[std::stoi(&ch)] = true;
        }
        break;
      case 'S':
        for (char ch : ruleStringElement)
        {
          rules.survive[std::stoi(&ch)] = true;
        }
        break;
      default:
        break;
      }
    }
    else if (std::regex_match(ruleString, SB_RE))
    {
      if (firstParse)
      {
        for (char ch : ruleStringElement)
        {
          rules.survive[std::stoi(&ch)] = true;
        }
        firstParse = false;
      }
      else
      {
        for (char ch : ruleStringElement)
        {
          rules.birth[std::stoi(&ch)] = true;
        }
      }
    }
  }
  if (rules.birth.size() == 0 || rules.survive.size() == 0)
  {
    // Initialize to Conway's Game of Life
    rRules.birth = std::array<bool, 9>{
      {false, false, false, true, false, false, false, false, false}};
    rRules.survive = std::array<bool, 9>{
      {false, false, true, true, false, false, false, false, false}};
  }
  rRules = rules;
}

void parser::_parseHeader(std::string header,
                          const BoardDimensions& maxDims,
                          GameOfLifeState& rState,
                          GameOfLifeRules& rRules)
{
  // Strip spaces
  header.erase(std::remove(header.begin(), header.end(), ' '), header.end());

  std::istringstream headerSS(header);
  BoardDimensions dims;
  for (std::string headerRule; std::getline(headerSS, headerRule, ',');)
  {
    std::istringstream headerRuleSS(headerRule);
    std::string lastToken;
    for (std::string headerRuleElement;
         std::getline(headerRuleSS, headerRuleElement, '=');)
    {
      if (lastToken == "x")
      {
        dims.x = std::stoul(headerRuleElement);
        if (dims.x > maxDims.x)
        {
          std::cerr << "Pattern width greater than max board width!"
                    << std::endl;
        }
      }
      else if (lastToken == "y")
      {
        dims.y = std::stoul(headerRuleElement);
        if (dims.y > maxDims.y)
        {
          std::cerr << "Pattern height greater than max board height!"
                    << std::endl;
        }
      }
      else if (lastToken == "rule")
      {
        _parseRuleString(headerRuleElement, rRules);
      }
      lastToken = headerRuleElement;
    }
  }
  rState.initialize(dims);
}

void parser::_parseBoard(std::string boardStr,
                         GameOfLifeState& rState,
                         std::string& rLastBoardLine,
                         int& currentRow,
                         bool last)
{
  boardStr = rLastBoardLine + boardStr;
  if (last)
  {
    // Pop off !
    boardStr.pop_back();
  }
  std::vector<std::string> rowStrs;
  std::istringstream boardSS(boardStr);
  for (std::string rowStr; std::getline(boardSS, rowStr, '$');)
  {
    rowStrs.push_back(rowStr);
  }
  if (!last)
  {
    // Don't process last string this round
    rLastBoardLine = rowStrs.back();
    rowStrs.pop_back();
  }
  std::string countStr;
  unsigned int count = 1;
  unsigned int col = 0;
  for (auto& rowStr : rowStrs)
  {
    for (auto ch : rowStr)
    {
      if (std::isdigit(ch))
      {
        countStr += ch;
      }
      else if (!countStr.empty())
      {
        count = std::stoul(countStr);
        countStr = std::string();
      }
      else
      {
        count = 1;
      }
      if (ch == 'o')
      {
        GolRow_t run(count, true);
        std::copy(
          run.begin(), run.end(), rState.board[currentRow].begin() + col);
        col += run.size();
      }
      else if (ch == 'b')
      {
        // Default is false
        col += count;
      }
    }
    col = 0;
    ++currentRow;
  }
}

bool parser::_placeStateOnBoard(const GameOfLifeState& referenceState,
                                GameOfLifeState& rTargetState)
{
  unsigned int x, y;
  // Top-left corner
  if (referenceState.dims.x <= rTargetState.dims.x)
  {
    x = (rTargetState.dims.x - referenceState.dims.x) / 2;
  }
  else
  {
    return false;
  }
  if (referenceState.dims.y <= rTargetState.dims.y)
  {
    y = (rTargetState.dims.y - referenceState.dims.y) / 2;
  }
  else
  {
    return false;
  }
  for (size_t j = 0; j < referenceState.board.size(); ++j)
  {
    std::copy(referenceState.board[j].begin(),
              referenceState.board[j].end(),
              rTargetState.board[y + j].begin() + x);
  }
  return true;
}

bool parser::parseRLE(std::string filename,
                      BoardDimensions dims,
                      GameOfLife& rGame)
{
  GameOfLife game;
  game.state.id = 0;
  game.state.initialize(dims);
  GameOfLifeState tempState;
  const std::regex COMMENT_RE(R"(#[CcNOPRr] .*)");
  const std::regex HEADER_RE(
    R"(x ?= ?\d+, ?y ?= ?\d+(, ?rule ?= ?(B\d+/S\d+|\d+/\d+)))");
  const std::regex NONTERMINAL_BODY_RE(R"([\dbo\$]+)");
  const std::regex TERMINAL_BODY_RE(R"([\dbo\$]*!)");

  std::string name;
  std::string author;
  std::string remainderBoardLine;
  ParseStage lastStage = ParseStage::NONE;

  int row = 0;
  std::stringstream fileSS(utils::readFile(filename));
  for (std::string line; std::getline(fileSS, line);)
  {
    // Strip carriage returns
    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
    if (std::regex_match(line, COMMENT_RE) &&
        (lastStage == ParseStage::COMMENT || lastStage == ParseStage::NONE))
    {
      _parseComment(line, game);
      lastStage = ParseStage::COMMENT;
    }
    else if (std::regex_match(line, HEADER_RE) &&
             (lastStage == ParseStage::NONE ||
              lastStage == ParseStage::COMMENT))
    {
      _parseHeader(line, game.state.dims, tempState, game.rules);
      lastStage = ParseStage::HEADER;
    }
    else if (std::regex_match(line, NONTERMINAL_BODY_RE) &&
             (lastStage == ParseStage::HEADER || lastStage == ParseStage::BODY))
    {
      _parseBoard(line, tempState, remainderBoardLine, row);
      lastStage = ParseStage::BODY;
    }
    else if (std::regex_match(line, TERMINAL_BODY_RE) &&
             (lastStage == ParseStage::HEADER || lastStage == ParseStage::BODY))
    {
      _parseBoard(line, tempState, remainderBoardLine, row, true);
      lastStage = ParseStage::COMPLETE;
    }
  }

  bool transferred = _placeStateOnBoard(tempState, game.state);
  rGame = game;
  return transferred && lastStage == ParseStage::COMPLETE;
}
