#ifndef H_GUARD_POSITION
#define H_GUARD_POSITION

#include "types.h"
#include <vector>

namespace Wyvern {

class Position {
private:
  U64 piece_colors[2];
  U64 ep_square;
  U64 pieces[6];
  enum CastlingRights castling;
  enum Color tomove;
  int fifty_half_moves;
  int full_moves;
  U64 zobrist;
  U64 zobrist2;
  std::vector<uint16_t> move_history;
  std::vector<enum PieceType> capture_history;
  std::vector<U64> position_history;
public:
  Position();
  int makeMove(U16 move);
  int unmakeMove();
  void printFen() const;
  void printPretty() const;
  int getHMC() const;
  int getFMC() const;
  U64* getPieceColors() const;
  U64* getPieces() const;
  enum Color getToMove() const;
};

}

#endif