#include "position.h"
#include <iostream>

namespace Wyvern {


int Position::makeMove(U16 move) {
  int isq = move & 0x3F;
  int tsq = (move >> 6) & 0x3F;
  int promo_piece = ((move >> 12) & 3) + 1; 
  int special = move & 0xC000;
  U64 ibb = 1ULL << isq;
  U64 tbb = 1ULL << tsq;
  int pt = 0;
  while (pt < 6 && !(pieces[pt] & ibb)) pt++;
  if (pt == 6 || !(piece_colors[tomove] & ibb)) {
    return 1;
  }
  int ptc = 0;
  while (ptc < 6 && !(pieces[ptc] & tbb)) ptc++;
  U64 ep_sq = 0;
  if (special == NORMAL) {
    piece_colors[tomove] ^= ibb ^ tbb;
    pieces[pt] ^= ibb ^ tbb;
    if (pt == (PAWN-1) && (ibb / tbb == 16 || tbb / ibb == 16))
      ep_square = ((ibb >> 8) & (tbb << 8)) | ((ibb << 8) & (tbb >> 8));
  }
  if (special == PROMO) {
    piece_colors[tomove] ^= ibb ^ tbb;
    pieces[promo_piece] ^= tbb;
  }

  // castling
  // en passant
  // TODO

  if (ptc < 6) {
    piece_colors[(tomove+1)&1] ^= tbb;
    pieces[ptc] ^= tbb;
  }
  move_history.emplace_back(move);
  capture_history.emplace_back((ptc + 1)%7);
  position_history.emplace_back(zobrist2);
}

enum Color Position::getToMove() const {
  return tomove;
}


}