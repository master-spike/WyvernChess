#ifndef H_GUARD_POSITION
#define H_GUARD_POSITION

#include <vector>
#include "types.h"


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
  std::vector<uint16_t> move_history;
  std::vector<enum PieceType> capture_history;
  std::vector<U64> position_history;
  
public:
  Position();
  ~Position() = default;
  int makeMove(U16 move);
  int unmakeMove();
  void printFen() const;
  void printPretty() const;
  int getHMC() const;
  int getFMC() const;
  enum CastlingRights getCR() const;
  U64* getPieceColors();
  U64* getPieces();
  enum Color getToMove() const;
  enum PieceType pieceAtSquare(U64 sq);
  U64 getEpSquare();
};

}

#endif
