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
  ep_square = 0;
  int ptc = 0;
  while (ptc < 6 && !(pieces[ptc] & tbb)) ptc++;
  if (special == NORMAL) {
    piece_colors[tomove] ^= ibb ^ tbb;
    pieces[pt] ^= ibb ^ tbb;
    if (pt == (PAWN-1) && (ibb >> 16 == tbb || tbb >> 16 == ibb))
      ep_square = ((ibb >> 8) & (tbb << 8)) | ((ibb << 8) & (tbb >> 8));
  }
  if (special == PROMO) {
    piece_colors[tomove] ^= ibb ^ tbb;
    pieces[promo_piece] ^= tbb;
    pieces[PAWN-1] ^= ibb;
  }
  if (special == CASTLES) {
    U64 castle_rank = (tomove == COLOR_WHITE) ? RANK_1 : RANK_8;
    U64 rook_isq = (ibb > tbb) ? castle_rank & FILE_A : castle_rank & FILE_B;
    U64 rook_tsq = (ibb > tbb) ? castle_rank & FILE_D : castle_rank & FILE_F;
    pieces[ROOK-1] ^= rook_isq ^ rook_tsq;
    pieces[KING-1] ^= ibb ^ tbb;
    piece_colors[tomove] ^= rook_isq ^ rook_tsq ^ ibb ^ tbb;
  }
  if (special == ENPASSANT) {
    pieces[PAWN-1] ^= ibb ^ tbb ^ ((RANK_4 | RANK_5) & (ep_square << 8 | ep_square >> 8));
    piece_colors[tomove] ^= ibb ^ tbb;
    piece_colors[tomove^1] ^= (ep_square << 8 | ep_square >> 8) & (RANK_4 | RANK_5);
  }
  if (ptc < 6) {
    piece_colors[tomove^1] ^= tbb;
    pieces[ptc] ^= tbb;
  }
  tomove = (enum Color) (tomove ^ 1);
  move_history.emplace_back(move);
  capture_history.emplace_back((enum PieceType) ((ptc + 1)%7));
  position_history.emplace_back(zobrist);
  return 0;
}

int Position::unmakeMove() {
  U16 move = move_history.back();

  enum PieceType captured_piece = capture_history.back();
  zobrist = position_history.back();
  tomove = (enum Color) (tomove ^ 1);
  int isq = move & 0x3F;
  int tsq = (move >> 6) & 0x3F;
  U64 ibb = 1ULL << isq;
  U64 tbb = 1ULL << tsq;
  int promo_piece = ((move >> 12) & 3) + 1; 
  int special = move & 0xC000;
  int pt = 0;
  ep_square = 0;
  while (pt < 6 && !(pieces[pt] & tbb)) pt++;
  if (pt == 6 || !(piece_colors[tomove] & tbb)) {
    tomove = (enum Color) (tomove ^ 1);
    return 1;
  }
  if (captured_piece != PIECE_NONE) {
    piece_colors[tomove^1] ^= tbb;
    pieces[captured_piece-1] ^= tbb;
  }
  if (special == NORMAL) {
    piece_colors[tomove] ^= ibb ^ tbb;
    pieces[pt] ^= ibb ^ tbb;
  }
  if (special == PROMO) {
    piece_colors[tomove] ^= ibb ^ tbb;
    pieces[promo_piece] ^= tbb;
    pieces[PAWN-1] ^= ibb;
  }
  if (special == CASTLES) {
    U64 castle_rank = (tomove == COLOR_WHITE) ? RANK_1 : RANK_8;
    U64 rook_isq = (ibb > tbb) ? castle_rank & FILE_A : castle_rank & FILE_B;
    U64 rook_tsq = (ibb > tbb) ? castle_rank & FILE_D : castle_rank & FILE_F;
    pieces[ROOK-1] ^= rook_isq ^ rook_tsq;
    pieces[KING-1] ^= ibb ^ tbb;
    piece_colors[tomove] ^= rook_isq ^ rook_tsq ^ ibb ^ tbb;
  }
  if (special == ENPASSANT) {
    ep_square = tbb;
    pieces[PAWN-1] ^= ibb ^ tbb ^ ((RANK_4 | RANK_5) & (ep_square << 8 | ep_square >> 8));
    piece_colors[tomove] ^= ibb ^ tbb;
    piece_colors[tomove^1] ^= (ep_square << 8 | ep_square >> 8) & (RANK_4 | RANK_5);
  }
  else if (move_history.size() > 0) {
    // RECOVER PREVIOUS EN PASSANT SQUARE
    U16 prev_move = move_history.back();
    int pm_t = 63 & (prev_move >> 6);
    int pm_i = prev_move & 63;
    if ((pieces[PAWN-1] & (1ULL << pm_t) && (pm_t - pm_i == 16 || pm_i - pm_t == 16)))
      ep_square = 1ULL << ((pm_t + pm_i) >> 2);
  }
  move_history.pop_back();
  capture_history.pop_back();
  position_history.pop_back();
  return 0;
}



enum Color Position::getToMove() const {
  return tomove;
}

Position::Position()
: piece_colors{0xFFFFULL, 0xFFFF000000000000ULL},
ep_square(0),
pieces{0xFF00000000FF00ULL, 0x4200000000000042ULL,
       0x2400000000000024ULL, 0x8100000000000081ULL,
       0x0800000000000008ULL, 0x1000000000000010ULL},
castling(CR_ANY),
tomove(COLOR_WHITE),
fifty_half_moves(0),
full_moves(0),
zobrist(0)
{
}

void Position::printFen() const {
  
}
void Position::printPretty() const {

}
int Position::getHMC() const {
  return fifty_half_moves;
}
int Position::getFMC() const {
  return full_moves;
}
enum CastlingRights Position::getCR() const {
  return castling;
}
U64* Position::getPieceColors() {
  return piece_colors;
}
U64* Position::getPieces() {
  return pieces;
}
enum PieceType Position::pieceAtSquare(U64 sq) {
  for (int i = 0; i < 6; i++) {
    if (sq & pieces[i]) return (enum PieceType) (i+1);
  }
  return PIECE_NONE;
}

U64 Position::getEpSquare() {
  return ep_square;
}

}
