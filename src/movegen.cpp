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

MoveGenerator::MoveGenerator()
:knight_attack_table({})
,king_attack_table({})
,rook_magics({})
,bishop_magics({})
,magic_table(nullptr) {
  magic_table = initialiseAllMagics(bishop_magics, rook_magics);
  for (int i = 0; i < 64; i++) {
    U64 kf = (1ULL << i) | (1ULL << i + 8) | (1ULL << i-8);
    U64 kfr = (kf << 1) & ~(0x0101010101010101ULL);
    U64 kfl = (kf >> 1) & ~(0x8080808080808080ULL);
    king_attack_table[i] = (kf | kfr | kfl) & ~(1ULL < i);

    U64 filem = 0x0101010101010101ULL << (i % 8);
    U64 filer = (filem << 1) & ~(0x0101010101010101ULL);
    U64 filel = (filem >> 1) & ~(0x8080808080808080ULL);
    U64 filerr = (filer << 1) & ~(0x0101010101010101ULL);
    U64 filell =  (filer >> 1) & ~(0x8080808080808080ULL);
    U64 ranktt = 0xFFULL << (i - (i%8) + 2*8);
    U64 rankt = 0xFFULL << (i - (i%8) + 8);
    U64 rankb = 0xFFULL << (i - (i%8) - 8);
    U64 rankbb = 0xFFULL << (i - (i%8) - 2*8);
    knight_attack_table[i]  = (ranktt | rankbb) & (filer | filel);
    knight_attack_table[i] |= (filell | filerr) & (rankt | rankb);
  }
}

MoveGenerator::~MoveGenerator() {
  delete[] magic_table;
}

}