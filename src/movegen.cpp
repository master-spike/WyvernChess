#include "movegen.h"


namespace Wyvern {

template<enum PieceType PT> U64 MoveGenerator::bbPseudoLegalMoves(int p, const U64& postmask, const U64& bb_blockers) {
  if constexpr (PT == KNIGHT) return knight_attack_table[p] & postmask;
  if constexpr (PT == BISHOP) return bishop_magics[p].compute(bb_blockers) & postmask;
  if constexpr (PT == ROOK) return rook_magics[p].compute(bb_blockers) & postmask;
  if constexpr (PT == KING) return king_attack_table[p] & postmask;
  if constexpr (PT == QUEEN) return (bishop_magics[p].compute(bb_blockers)| rook_magics[p].compute(bb_blockers)) & postmask;
  return 0;
}

template<enum Color CT> U64 MoveGenerator::bbPawnSinglePushes(const U64& pawns, const U64& postmask) {
  if constexpr (CT == COLOR_WHITE) return (pawns << 8) & postmask;
  if constexpr (CT == COLOR_BLACK) return (pawns >> 8) & postmask;
  return 0; 
}
template<enum Color CT> U64 MoveGenerator::bbPawnDoublePushes(const U64& pawns, const U64& postmask, const U64& bb_blockers) {
  if constexpr (CT == COLOR_WHITE) {
    return ((pawns & 0xFF00ULL) << 16) & postmask & (~(bb_blockers << 8));
  }
  if constexpr (CT == COLOR_BLACK)  {
    return ((pawns & 0xFF000000000000ULL) >> 16) & postmask & (~(bb_blockers >> 8));
  }
  return 0; 
}
template<enum Color CT> U64 MoveGenerator::bbPawnLeftCaptures(const U64& pawns, const U64& postmask, const U64& capturables) {
  if constexpr (CT == COLOR_WHITE) {
    return (pawns << 7) & postmask & capturables & ~(0x0101010101010101ULL);
  }
  if constexpr (CT == COLOR_BLACK) {
    return (pawns >> 7) & postmask & capturables & ~(0x8080808080808080ULL);
  }
  return 0;
}
template<enum Color CT> U64 MoveGenerator::bbPawnRightCaptures(const U64& pawns, const U64& postmask, const U64& capturables) {
  if constexpr (CT == COLOR_WHITE) {
    return (pawns << 9) & postmask & capturables & ~(0x8080808080808080ULL);
  }
  if constexpr (CT == COLOR_BLACK) {
    return (pawns >> 9) & postmask & capturables & ~(0x0101010101010101ULL);
  }
  return 0;
}

}