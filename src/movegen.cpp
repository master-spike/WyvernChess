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
    U64 kattack_through_p = bishop_magics[king].compute(bl_m_p);
    // pinner can have at most 1 bit set because of math!
    U64 pinner = bishop_magics[p].compute(blockers)
                 & kattack_through_p
                 & (enemy_diag);
    if (pinner) pinmask = kattack_through_p & (bishop_magics[__builtin_ctzll(pinner)].compute(bl_m_p) | pinner);
  }
  else if (pp_o & p_bit) {
    U64 kattack_through_p = rook_magics[king].compute(bl_m_p);
    U64 pinner = rook_magics[p].compute(blockers)
                 & kattack_through_p
                 & (enemy_orth);
    if (pinner) pinmask = kattack_through_p & (rook_magics[__builtin_ctzll(pinner)].compute(bl_m_p) | pinner);
  }
  return pinmask;
}

U16 MoveGenerator::popMove() {
  for (int i = 0; i < 3; ++i) {
    if (!categorized_moves[i].empty()) {
      U16 move = categorized_moves[i].back();
      categorized_moves[i].pop_back();
      if (move != MOVE_NONE) return move;
    }
  }
  return MOVE_NONE;
}

void MoveGenerator::flushMoves() {
  categorized_moves[0].clear();
  categorized_moves[1].clear();
  categorized_moves[2].clear();
}

MoveGenerator::MoveGenerator()
:knight_attack_table{}
,king_attack_table{}
,rook_magics{}
,bishop_magics{}
,magic_table(nullptr) {
  magic_table = initialiseAllMagics(bishop_magics, rook_magics);
  for (int i = 0; i < 64; i++) {
    U64 kf = (1ULL << i);
    if (i >= 8) kf |= kf >> 8;
    if (i < 56) kf |= kf << 8;
    U64 kfr = (kf << 1) & ~(FILE_A);
    U64 kfl = (kf >> 1) & ~(FILE_H);
    king_attack_table[i] = (kf | kfr | kfl) & ~(1ULL << i);

    U64 filem = FILE_A << (i % 8);
    U64 filer = (filem << 1) & ~(FILE_A);
    U64 filel = (filem >> 1) & ~(FILE_H);
    U64 filerr = (filer << 1) & ~(FILE_A);
    U64 filell =  (filer >> 1) & ~(FILE_H);
    U64 ranktt = (i / 8 < 6) ? ranks[i / 8 + 2] : 0;
    U64 rankt = (i / 8 < 7) ? ranks[i / 8 + 1] : 0;
    U64 rankb = (i / 8 > 0) ? ranks[i / 8 - 1] : 0;
    U64 rankbb = (i / 8 > 1) ? ranks[i / 8 - 2] : 0;
    knight_attack_table[i]  = (ranktt | rankbb) & (filer | filel);
    knight_attack_table[i] |= (filell | filerr) & (rankt | rankb);
  }
}

MoveGenerator::~MoveGenerator() {
  delete[] magic_table;
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

template int MoveGenerator::generateMoves<COLOR_BLACK>(Position& pos);
template int MoveGenerator::generateMoves<COLOR_WHITE>(Position& pos);

}
