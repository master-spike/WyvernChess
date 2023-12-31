#ifndef H_GUARD_POSITION
#define H_GUARD_POSITION

#include <bit>
#include <vector>

#include "types.h"
#include "utils.h"
#include "zobrist.h"


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
  Position(const Position& pos);
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
  bool operator == (Position& pos) {
    //if (zobrist ^ pos.zobrist) return false;
    if (piece_colors[0] ^ pos.piece_colors[0]) return false;
    if (piece_colors[1] ^ pos.piece_colors[1]) return false;
    for (int i = 0; i < 6; ++i) {
      if (pieces[i] ^ pos.pieces[i]) return false;
    }
    //if (ep_square ^ pos.ep_square) return false;
    if (castling != pos.castling) return false;
    if (tomove != pos.tomove) return false;
    //if (fifty_half_moves != pos.fifty_half_moves) return false;
    //if (full_moves != pos.full_moves) return false;
    return true;
  }
};

}

#endif
