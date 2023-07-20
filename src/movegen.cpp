#include "movegen.h"
#include <iostream>

namespace Wyvern {

// responsibility on caller to calculate pp_d and pp_o which are possi1ULL <bly pinned pieces diag and ortho resp.
U64 MoveGenerator::makePinmask(int p, U64 pp_d, U64 pp_o, U64 blockers, int king, U64 enemy_diag, U64 enemy_orth) {
  U64 p_bit = 1ULL << p;
  U64 pinmask = 0xFFFFFFFFFFFFFFFFULL;
  U64 bl_m_p = blockers & ~p_bit;
  if (pp_d & p_bit) {
    U64 bl_m_p = blockers & ~p_bit;
    U64 kattack_through_p = mt->bishop_magics[king].compute(bl_m_p);
    // pinner can have at most 1 bit set because of math!
    U64 pinner = mt->bishop_magics[p].compute(blockers)
                 & kattack_through_p
                 & (enemy_diag);
    if (pinner) pinmask = kattack_through_p & (mt->bishop_magics[__builtin_ctzll(pinner)].compute(bl_m_p) | pinner);
  }
  else if (pp_o & p_bit) {
    U64 kattack_through_p = mt->rook_magics[king].compute(bl_m_p);
    U64 pinner = mt->rook_magics[p].compute(blockers)
                 & kattack_through_p
                 & (enemy_orth);
    if (pinner) pinmask = kattack_through_p & (mt->rook_magics[__builtin_ctzll(pinner)].compute(bl_m_p) | pinner);
  }
  return pinmask;
}

/*
int moveRating(U32 move) {
  if (move & YES_CAPTURE) {
    move >> 
  }
}
*/

void MoveGenerator::flushMoves() {
  move_targets->clear();
}


U64 MoveGenerator::inCheck(Position& pos) {
  enum Color tomove = pos.getToMove();
  if (tomove >= 2) return 0;
  int myKing = __builtin_ctzll(pos.getPieces()[KING-1] & pos.getPieceColors()[tomove]);
  if (tomove) return squareAttackedBy<COLOR_WHITE>(myKing, pos, 0);
  else return squareAttackedBy<COLOR_BLACK>(myKing, pos, 0);
}

MoveGenerator::MoveGenerator(std::shared_ptr<MagicTable> _mt) {
  mt = std::shared_ptr<MagicTable>(_mt);
}


template U64 MoveGenerator::bbCastles<COLOR_BLACK>(Position& pos);
template U64 MoveGenerator::bbCastles<COLOR_WHITE>(Position& pos);

template U64 MoveGenerator::squareAttackedBy<COLOR_BLACK>(int p, Position& pos, U64);
template U64 MoveGenerator::squareAttackedBy<COLOR_WHITE>(int p, Position& pos, U64);

template U64 MoveGenerator::bbPseudoLegalMoves<PAWN, COLOR_WHITE>(int p, U64 postmask, U64 bb_blockers);
template U64 MoveGenerator::bbPseudoLegalMoves<KNIGHT, COLOR_WHITE>(int p, U64 postmask, U64 bb_blockers);
template U64 MoveGenerator::bbPseudoLegalMoves<BISHOP, COLOR_WHITE>(int p, U64 postmask, U64 bb_blockers);
template U64 MoveGenerator::bbPseudoLegalMoves<ROOK, COLOR_WHITE>(int p, U64 postmask, U64 bb_blockers);
template U64 MoveGenerator::bbPseudoLegalMoves<QUEEN, COLOR_WHITE>(int p, U64 postmask, U64 bb_blockers);
template U64 MoveGenerator::bbPseudoLegalMoves<PAWN, COLOR_BLACK>(int p, U64 postmask, U64 bb_blockers);
template U64 MoveGenerator::bbPseudoLegalMoves<KNIGHT, COLOR_BLACK>(int p, U64 postmask, U64 bb_blockers);
template U64 MoveGenerator::bbPseudoLegalMoves<BISHOP, COLOR_BLACK>(int p, U64 postmask, U64 bb_blockers);
template U64 MoveGenerator::bbPseudoLegalMoves<ROOK, COLOR_BLACK>(int p, U64 postmask, U64 bb_blockers);
template U64 MoveGenerator::bbPseudoLegalMoves<QUEEN, COLOR_BLACK>(int p, U64 postmask, U64 bb_blockers);

template void MoveGenerator::generateStandardMoves<PAWN, COLOR_WHITE>(U64, U64, U64, int, U64, U64, U64, U64, U64, U64, U64, U64);
template void MoveGenerator::generateStandardMoves<KNIGHT, COLOR_WHITE>(U64, U64, U64, int, U64, U64, U64, U64, U64, U64, U64, U64);
template void MoveGenerator::generateStandardMoves<BISHOP, COLOR_WHITE>(U64, U64, U64, int, U64, U64, U64, U64, U64, U64, U64, U64);
template void MoveGenerator::generateStandardMoves<ROOK, COLOR_WHITE>(U64, U64, U64, int, U64, U64, U64, U64, U64, U64, U64, U64);
template void MoveGenerator::generateStandardMoves<QUEEN, COLOR_WHITE>(U64, U64, U64, int, U64, U64, U64, U64, U64, U64, U64, U64);
template void MoveGenerator::generateStandardMoves<PAWN, COLOR_BLACK>(U64, U64, U64, int, U64, U64, U64, U64, U64, U64, U64, U64);
template void MoveGenerator::generateStandardMoves<KNIGHT, COLOR_BLACK>(U64, U64, U64, int, U64, U64, U64, U64, U64, U64, U64, U64);
template void MoveGenerator::generateStandardMoves<BISHOP, COLOR_BLACK>(U64, U64, U64, int, U64, U64, U64, U64, U64, U64, U64, U64);
template void MoveGenerator::generateStandardMoves<ROOK, COLOR_BLACK>(U64, U64, U64, int, U64, U64, U64, U64, U64, U64, U64, U64);
template void MoveGenerator::generateStandardMoves<QUEEN, COLOR_BLACK>(U64, U64, U64, int, U64, U64, U64, U64, U64, U64, U64, U64);

template int MoveGenerator::generateMoves<COLOR_BLACK>(Position&, bool, std::vector<U32>*);
template int MoveGenerator::generateMoves<COLOR_WHITE>(Position&, bool, std::vector<U32>*);

}
