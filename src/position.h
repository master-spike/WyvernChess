#pragma once

#include <array>
#include <bit>
#include <vector>

#include "types.h"
#include "utils.h"
#include "zobrist.h"

namespace Wyvern
{

class Position
{
private:
  std::array<U64, 2> piece_colors;
  U64 ep_square;
  std::array<U64, 6> pieces;
  enum CastlingRights castling;
  enum Color tomove;
  int fifty_half_moves;
  int full_moves;
  U64 zobrist;
  std::vector<U64> position_history;
  std::vector<enum CastlingRights> cr_history;
  std::vector<int> hmc_history;

public:
  std::vector<U64>::const_reverse_iterator positionHistoryIteratorBegin() const;
  std::vector<U64>::const_reverse_iterator positionHistoryIteratorEnd() const;
  std::vector<U32> move_history;
  Position();
  Position(const char* fen);
  ~Position() = default;
  Position(const Position& pos) = default;
  void zobristHash();
  int makeMove(U32 move);
  int unmakeMove();
  void printFen();
  void printPretty();
  int getHMC() const;
  int getFMC() const;
  U64 getZobrist() const;
  enum CastlingRights getCR() const;
  const U64* getPieceColors() const;
  const U64* getPieces() const;
  enum Color getToMove() const;
  int checkValidity();
  enum PieceType pieceAtSquare(U64 sq);
  U64 getEpSquare();
  bool operator==(const Position& pos) const
  {
    return piece_colors == pos.piece_colors && pieces == pos.pieces && castling == pos.castling &&
           tomove == pos.tomove;
  }
};

} // namespace Wyvern
