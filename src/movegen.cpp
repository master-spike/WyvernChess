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
    return ((pawns << 7) & postmask & capturables & ~(0x0101010101010101ULL)) >> 7;
  }
  if constexpr (CT == COLOR_BLACK) {
    return ((pawns >> 7) & postmask & capturables & ~(0x8080808080808080ULL)) << 7;
  }
  return 0;
}
template<enum Color CT> U64 MoveGenerator::bbPawnRightCaptures(const U64& pawns, const U64& postmask, const U64& capturables) {
  if constexpr (CT == COLOR_WHITE) {
    return ((pawns << 9) & postmask & capturables & ~(0x8080808080808080ULL)) >> 9;
  }
  if constexpr (CT == COLOR_BLACK) {
    return ((pawns >> 9) & postmask & capturables & ~(0x0101010101010101ULL)) << 9;
  }
  return 0;
}

template<enum Color CT> U64 MoveGenerator::bbCastles(const Position& pos, const U64& checkmask) {
  enum CastlingRights cr = pos.getCR() & ((CT = COLOR_WHITE) ? CR_WHITE : CR_BLACK);
  if (checkmask) return 0;
  U64 qs_path = 0x0EULL << (56 * CT);
  U64 ks_path = 0x60ULL << (56 * CT);
  U64 blockers = pos.getPieceColors[0];
  blockers |= pos.getPieceColors[1];
  int king = __builtin_ctzll(pos.getPieces[KING-1] & pos.getPieceColors[CT]);
  U64 out_bb = 0;
  if ((cr & CR_QUEEN) && !(qs_path & blockers)) {
    if (!squareAttackedBy<CT ^ 1>(king-1, pos) && !squareAttackedBy<CT ^ 1>(king-2, pos)) {
      out_bb |= 1ULL << (king-2);
    }
  }
  if ((cr & CR_KING) && !(ks_path & blockers)) {
    if (!squareAttackedBy<CT ^ 1>(king+1, pos) && !squareAttackedBy<CT ^ 1>(king+2, pos)) {
      out_bb |= 1ULL << (king+2);
    }
  }
  return out_bb;
}

template<enum Color CT> // CT is attacker
U64 MoveGenerator::squareAttackedBy(int p, const Position& pos, U64 custom_blockers = 0) {
  U64* pcols = pos.getPieceColors();
  U64* pcs_atk = pos.getPieces() + CT*6;
  U64 blockers = (custom_blockers) ? custom_blockers : pcols[0] | pcols[1];
  U64 potential_attackers = pcols[CT];
  U64 attackers = 0;
  attackers |= pcs_atk[QUEEN-1] & (bishop_magics[p].compute(blockers) | rook_magics[p].compute(blockers));
  attackers |= pcs_atk[ROOK-1] & (rook_magics[p].compute(blockers));
  attackers |= pcs_atk[BISHOP-1] & (bishop_magics[p].compute(blockers));
  attackers |= pcs_atk[KNIGHT-1] & knight_attack_table[p];
  attackers |= pcs_atk[KING-1] & king_attack_table[p];
  attackers |= bbPawnLeftCaptures<CT^1>(1ULL << p, king_attack_table[p], pcs_atk[PAWN-1]);
  attackers |= bbPawnRightCaptures<CT^1>(1ULL << p, king_attack_table[p], pcs_atk[PAWN-1]);
}

// responsibility on caller to calculate pp_d and pp_o which are possibly pinned pieces diag and ortho resp.
U64 MoveGenerator::makePinmask(int p, U64 pp_d, U64 pp_o, U64 blockers, int king, U64 enemy_diag, U64 enemy_orth) {
  U64 p_bit = 1ULL << p;
  U64 pinmask = 0xFFFFFFFFFFFFFFFFULL;
  U64 bl_m_p = blockers & ~p_bit;
  if (pp_d & p_bit) {
    U64 bl_m_p = blockers & ~p_bit;
    U64 kattack_through_p = bishop_magics[king].compute(bl_m_p);
    // pinner can have at most 1 bit set because of math!
    U64 pinner = bishop_magics[p].compute(blockers)
                 & kattack_through_p;
                 & (enemy_diag);
    if (pinner) pinmask = kattack_through_p & (bishop_magics[__builtin_ctzll(pinner)].compute(bl_m_p) | pinner);
  }
  else if (pp_o & p_bit) {
    U64 kattack_through_p = rook_magics[king].compute(bl_m_p);
    U64 pinner = rook_magics[p].compute(blockers)
                 & kattack_through_p;
                 & (enemy_orth);
    if (pinner) pinmask = kattack_through_p & (rook_magics[__builtin_ctzll(pinner)].compute(bl_m_p) | pinner);
  }
  return pinmask;
}

template<enum Color CT> int MoveGenerator::generateMoves(Position& pos, U16* outMoves) {
  if constexpr (CT > 1) return 0;
  U64 pieces[6];
  U64 piece_colors[2];
  {
    U64* pcls = pos.getPieceColors();
    U64* pcs = pos.getPieces();
    for (int i = 0; i < 6; i++) pieces[i] = pcs[i];
    piece_colors[0] = pcls[0];
    piece_colors[1] = pcls[1];
  }
  U64 blockers = piece_colors[0] | piece_colors[1];
  int myking = __builtin_ctzll(pieces[KING-1] & piece_colors[CT]); 
  U64 checkmask = 0xFFFFFFFFFFFFFFFFULL; // sneaky all bits set
  U64 checkers = squareAttackedBy<CT^1>(myking, pos);
  if (__builtin_popcountll(checkers) > 1) checkmask = 0; // if double check, we can skip all non-king moves
  else if (checkers) {
    enum Piece checker_piece = pos.pieceAtSquare(checkers);
    switch(checker_piece) {
      case PAWN:
        checkmask = checker_piece;
      case KNIGHT:
        checkmask = checker_piece;
        break;
      default:
        checkmask = boardRay(__builtin_ctzll(checkers), myking) & ~(1ULL << myking);
    }
  }
  int move_count = 0;
  // checkmask now holds all possible target squares to block or capture a checking piece if applicable
  if (checkmask) {
    U64 possibly_pinned_d = bishop_magics[myking].compute(blockers);
    U64 possibly_pinned_o = rook_magics[myking].compute(blockers);
    U64 enemy_queens = pieces[QUEEN-1] & piece_colors[CT^1];
    U64 enemy_rooks = pieces[ROOK-1] & piece_colors[CT^1];
    U64 enemy_bishops = pieces[BISHOP-1] & piece_colors[CT^1];
    U64 enemy_knights = pieces[KNIGHT-1] & piece_colors[CT^1];
    U64 enemy_pawns = pieces[PAWN-1] & piece_colors[CT^1];
    U64 empty_space = ~(blockers);
    
    // do pawn captures of all pieces first, in order of piece value.
    U64 pxql = bbPawnLeftCaptures<CT>(pieces[PAWN-1] & piece_colors[CT], checkmask, enemy_queens);
    for (; pxql; pxql = pxql & (pxql - 1)) {
      int p = __builtin_ctzll(p);
    }
    U64 pxqr = bbPawnRightCaptures<CT>(pieces[PAWN-1] & piece_colors[CT], checkmask, enemy_queens);
    U64 pxrl = bbPawnLeftCaptures<CT>(pieces[PAWN-1] & piece_colors[CT], checkmask, enemy_rooks);
    U64 pxrr = bbPawnRightCaptures<CT>(pieces[PAWN-1] & piece_colors[CT], checkmask, enemy_rooks);
    U64 pxbnl = bbPawnLeftCaptures<CT>(pieces[PAWN-1] & piece_colors[CT], checkmask, enemy_bishops | enemy_knights);
    U64 pxbnr = bbPawnRightCaptures<CT>(pieces[PAWN-1] & piece_colors[CT], checkmask, enemy_bishops | enemy_knights);
    


  }
  // king moves

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