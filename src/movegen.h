#ifndef H_GUARD_MOVEGEN
#define H_GUARD_MOVEGEN

#include "position.h"
#include "types.h"
#include "magicbb.h"
#include "utils.h"
#include <array>
#include <vector>

#define MG_NUM_MOVELISTS 1

namespace Wyvern {


/*

move generator is responsible for putting valid moves in a Position into a move vector<U32> in preference order.
search can then pop these moves.
*/

class MoveGenerator {
private:
  template<enum PieceType PT, enum Color CT> U64 bbPseudoLegalMoves(int p, U64 postmask, U64 bb_blockers);
  template<enum Color CT> U64 bbCastles(Position& pos);
  U64 makePinmask(int p, U64 pp_d, U64 pp_o, U64 blockers, int king, U64 enemy_diag, U64 enemy_orth);
  template<enum PieceType PT, enum Color CT>
  void generateStandardMoves(U64 ps, U64 checkmask, U64 blockers, int myking, U64 our_pieces,
                             U64 e_p, U64 e_n, U64 e_b, U64 e_r, U64 e_q, U64 pp_o, U64 pp_d);
  void flushMoves();
  void emplaceCaptures(int p, enum PieceType pt, U64 e_p, U64 e_n, U64 e_b, U64 e_r, U64 e_q, U64 targets) {
    for(U64 t = targets & e_p; t; t &= t - 1) {
      int tp = __builtin_ctzll(t);
      generated_moves.emplace_back(p + (tp << 6) + CAPTURE_PAWN + ((U32)pt << 20));
    }
    for(U64 t = targets & e_n; t; t &= t - 1) {
      int tp = __builtin_ctzll(t);
      generated_moves.emplace_back(p + (tp << 6) + CAPTURE_KNIGHT + ((U32)pt << 20));
    }
    for(U64 t = targets & e_b; t; t &= t - 1) {
      int tp = __builtin_ctzll(t);
      generated_moves.emplace_back(p + (tp << 6) + CAPTURE_BISHOP + ((U32)pt << 20));
    }
    for(U64 t = targets & e_r; t; t &= t - 1) {
      int tp = __builtin_ctzll(t);
      generated_moves.emplace_back(p + (tp << 6) + CAPTURE_ROOK + ((U32)pt << 20));
    }
    for(U64 t = targets & e_q; t; t &= t - 1) {
      int tp = __builtin_ctzll(t);
      generated_moves.emplace_back(p + (tp << 6) + CAPTURE_QUEEN + ((U32)pt << 20));
    }
  }
  void emplaceNonCaptures(int p, enum PieceType pt, U64 targets) {
    for(U64 t = targets; t; t &= t-1) {
      int tp = __builtin_ctzll(t);
      generated_moves.emplace_back(p + (tp << 6) + ((U32)pt << 20));
    }
  }
  void emplacePromotions(int p, U64 e_p, U64 e_n, U64 e_b, U64 e_r, U64 e_q, U64 targets) {
    for (U64 t = targets; t; t&= t-1) {
      int tp = __builtin_ctzll(t);
      enum MoveCapture cap = NO_CAPTURE;
      if (e_p & (1ULL << tp)) cap = CAPTURE_PAWN;
      if (e_b & (1ULL << tp)) cap = CAPTURE_BISHOP;
      if (e_n & (1ULL << tp)) cap = CAPTURE_KNIGHT;
      if (e_r & (1ULL << tp)) cap = CAPTURE_ROOK;
      if (e_q & (1ULL << tp)) cap = CAPTURE_QUEEN;
      for (U32 pp = 0; pp <= (3 << 12); pp += 1 << 12) {
        generated_moves.emplace_back(p + (tp << 6) + ((U32)cap) + PROMO + pp + MOVE_PAWN);
      }
    }
  }
  std::vector<U32> generated_moves;
public:
  U64 knight_attack_table[64];
  U64 king_attack_table[64];
  MagicBB rook_magics[64];
  MagicBB bishop_magics[64];
  U64* magic_table;
  template<enum Color CT> U64 squareAttackedBy(int p, Position& pos, U64 custom_blockers);
  MoveGenerator();
  MoveGenerator(const MoveGenerator& in_mg) = delete;
  template<enum Color CT> int generateMoves(Position& pos);
  U64 inCheck(Position& pos);
  ~MoveGenerator();
  U32 popMove(int order);
};

template<enum PieceType PT, enum Color CT>
U64 MoveGenerator::bbPseudoLegalMoves(int p, U64 postmask, U64 bb_blockers) {
  if constexpr (PT == KNIGHT) return knight_attack_table[p] & postmask;
  if constexpr (PT == BISHOP) return bishop_magics[p].compute(bb_blockers) & postmask;
  if constexpr (PT == ROOK) return rook_magics[p].compute(bb_blockers) & postmask;
  if constexpr (PT == KING) return king_attack_table[p] & postmask;
  if constexpr (PT == QUEEN) return (bishop_magics[p].compute(bb_blockers) | rook_magics[p].compute(bb_blockers)) & postmask;
  if constexpr (PT == PAWN) {
    if constexpr (CT == COLOR_WHITE) {
      U64 pawn = 1ULL << p;
      U64 captures = ((pawn << 7 & ~FILE_H) | (pawn << 9 & ~FILE_A)) & bb_blockers;
      U64 pushes = (pawn << 8) & ~bb_blockers;
      pushes |= (pushes << 8) & RANK_4 & ~bb_blockers;
      return (captures | pushes) & postmask;
    }
    if constexpr (CT == COLOR_BLACK) {
      U64 pawn = 1ULL << p;
      U64 captures = ((pawn >> 9 & ~FILE_H) | (pawn >> 7 & ~FILE_A)) & bb_blockers;
      U64 pushes = (pawn >> 8) & ~bb_blockers;
      pushes |= (pushes >> 8) & RANK_5 & ~bb_blockers;
      return (captures | pushes) & postmask;
    }
  }
  return 0;
}

template<enum Color CT> U64 MoveGenerator::bbCastles(Position& pos) {
  constexpr enum Color CTO = (enum Color) (CT^1);
  enum CastlingRights cr = (enum CastlingRights) (pos.getCR() & ((CT == COLOR_WHITE) ? CR_WHITE : CR_BLACK));
  U64 qs_path = 0x0EULL << (56 * CT);
  U64 ks_path = 0x60ULL << (56 * CT);
  U64 blockers = pos.getPieceColors()[0];
  blockers |= pos.getPieceColors()[1];
  int king = __builtin_ctzll(pos.getPieces()[KING-1] & pos.getPieceColors()[CT]);
  U64 out_bb = 0;
  if ((cr & CR_QUEEN) && !(qs_path & blockers)) {
    if (!squareAttackedBy<CTO>(king-1, pos, 0) && !squareAttackedBy<CTO>(king-2, pos, 0)) {
      out_bb |= 1ULL << (king-2);
    }
  }
  if ((cr & CR_KING) && !(ks_path & blockers)) {
    if (!squareAttackedBy<CTO>(king+1, pos, 0) && !squareAttackedBy<CTO>(king+2, pos, 0)) {
      out_bb |= 1ULL << (king+2);
    }
  }
  return out_bb;
}

template<enum Color CT>
U64 MoveGenerator::squareAttackedBy(int p, Position& pos, U64 custom_blockers) {
  U64* pcols = pos.getPieceColors();
  U64* pcs = pos.getPieces();
  U64 blockers = (custom_blockers) ? custom_blockers : (pcols[0] | pcols[1]) & ~(1ULL << p);
  U64 attackers = 0;
  attackers |= pcs[QUEEN-1] & (bishop_magics[p].compute(blockers) | rook_magics[p].compute(blockers));
  attackers |= pcs[ROOK-1] &(rook_magics[p].compute(blockers));
  attackers |= pcs[BISHOP-1] & (bishop_magics[p].compute(blockers));
  attackers |= pcs[KNIGHT-1] & knight_attack_table[p];
  attackers |= pcs[KING-1] & king_attack_table[p];
  if constexpr (CT == COLOR_WHITE) {
    attackers |= pcs[PAWN-1] & ((1ULL << (p - 7) & ~FILE_A) | (1ULL << (p - 9) & ~FILE_H));
  }
  if constexpr (CT == COLOR_BLACK) {
    attackers |= pcs[PAWN-1] & ((1ULL << (p + 9) & ~FILE_A) | (1ULL << (p + 7) & ~FILE_H));
  }
  attackers &= pcols[CT];
  return attackers;
}


//not to be used for king moves
template<enum PieceType PT, enum Color CT>
void MoveGenerator::generateStandardMoves(U64 ps, U64 checkmask, U64 blockers, int myking, U64 our_pieces, U64 e_p, U64 e_n, U64 e_b, U64 e_r, U64 e_q, U64 pp_o, U64 pp_d) {
  U64 caps = e_p | e_n | e_b | e_r | e_q;
  if constexpr (PT == PAWN) {
    U64 promo_rank = RANK_8 >> (56*CT);
    for (; ps; ps &= ps-1) {
      int p = __builtin_ctzll(ps);
      U64 pinmask = makePinmask(p, pp_d, pp_o, blockers, myking, (e_q|e_b), (e_q|e_r));
      U64 targets = bbPseudoLegalMoves<PAWN, CT>(p, checkmask & pinmask & ~our_pieces, blockers);
      U64 t_promotions = targets & promo_rank;
      U64 t_captures = targets & caps & ~promo_rank;
      U64 t_non_caps = targets & ~caps & ~promo_rank;
      emplacePromotions(p, e_p, e_n, e_b, e_r, e_q, t_promotions);
      emplaceCaptures(p, PAWN, e_p, e_n, e_b, e_r, e_q, t_captures);
      emplaceNonCaptures(p, PAWN, t_non_caps);
    }
  }
  else if constexpr (PT >= KNIGHT && PT <= QUEEN) {
    for (;ps; ps &= ps-1) {
      int p = __builtin_ctzll(ps);  
      U64 pinmask = makePinmask(p, pp_d, pp_o, blockers, myking, (e_q|e_b), (e_q|e_r));
      U64 targets = bbPseudoLegalMoves<PT, CT>(p, checkmask & pinmask & ~our_pieces, blockers);
      U64 t_caps = targets & caps;
      U64 t_quiets = targets & ~caps;
      emplaceCaptures(p, PT, e_p, e_n, e_b, e_r, e_q, t_caps);
      emplaceNonCaptures(p, PT, t_quiets);
    }
  }
}

template<enum Color CT>
int MoveGenerator::generateMoves(Position& pos) {

  constexpr enum Color CTO = (enum Color) (CT ^ 1); 
  flushMoves();
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

  U64 checkers = squareAttackedBy<CTO>(myking, pos, 0);
  U64 king_file = files[myking % 8];
  U64 king_rank = ranks[myking / 8];

  if (__builtin_popcountll(checkers) > 1) checkmask = 0; // if double check, we can skip all non-king moves
  else if (checkers) {
    enum PieceType checker_piece = pos.pieceAtSquare(checkers);
    switch(checker_piece) {
      case PAWN:
        checkmask = checkers;
        break;
      case KNIGHT:
        checkmask = checkers;
        break;
      default:
        if ((king_file ^ king_rank) & checkers) {
          checkmask = rook_magics[__builtin_ctzll(checkers)].compute(blockers)
                    & rook_magics[myking].compute(blockers);
        }
        else {
          checkmask = bishop_magics[__builtin_ctzll(checkers)].compute(blockers)
                    & bishop_magics[myking].compute(blockers);
        }
    }
    checkmask |= checkers;
    //std::cout << "in (single) check - here is position and checkmask:\n";
    //pos.printPretty();
    //printbb(checkmask);
  }
  // checkmask now holds all possible target squares to block or capture a checking piece if applicable
  U64 enemy_queens = pieces[QUEEN-1] & piece_colors[CTO];
  U64 enemy_rooks = pieces[ROOK-1] & piece_colors[CTO];
  U64 enemy_bishops = pieces[BISHOP-1] & piece_colors[CTO];
  U64 enemy_knights = pieces[KNIGHT-1] & piece_colors[CTO];
  U64 enemy_pawns = pieces[PAWN-1] & piece_colors[CTO];

  //printbb(checkmask);
  if (checkmask) {
    U64 pp_d = bishop_magics[myking].compute(blockers);
    U64 pp_o = rook_magics[myking].compute(blockers);
    // move ordering:
    // pxq, pxr, px
    generateStandardMoves<PAWN,CT>(piece_colors[CT] & pieces[PAWN-1], checkmask, blockers, myking,
                                   piece_colors[CT], enemy_pawns, enemy_knights, enemy_bishops,
                                   enemy_rooks, enemy_queens, pp_o, pp_d);
    generateStandardMoves<KNIGHT,CT>(piece_colors[CT] & pieces[KNIGHT-1], checkmask, blockers, myking,
                                   piece_colors[CT], enemy_pawns, enemy_knights, enemy_bishops,
                                   enemy_rooks, enemy_queens, pp_o, pp_d);
    generateStandardMoves<BISHOP,CT>(piece_colors[CT] & pieces[BISHOP-1], checkmask, blockers, myking,
                                   piece_colors[CT], enemy_pawns, enemy_knights, enemy_bishops,
                                   enemy_rooks, enemy_queens, pp_o, pp_d);
    generateStandardMoves<ROOK,CT>(piece_colors[CT] & pieces[ROOK-1], checkmask, blockers, myking,
                                   piece_colors[CT], enemy_pawns, enemy_knights, enemy_bishops,
                                   enemy_rooks, enemy_queens, pp_o, pp_d);
    generateStandardMoves<QUEEN,CT>(piece_colors[CT] & pieces[QUEEN-1], checkmask, blockers, myking,
                                   piece_colors[CT], enemy_pawns, enemy_knights, enemy_bishops,
                                   enemy_rooks, enemy_queens, pp_o, pp_d);

  }
  // king moves
  U64 pseudo_king_moves = bbPseudoLegalMoves<KING, CT>(myking, ~piece_colors[CT], blockers);
  for (U64 tt = pseudo_king_moves; tt; tt &= tt-1) {
    int t = __builtin_ctzll(tt);
    if (squareAttackedBy<CTO>(t, pos, blockers ^ (1ULL << myking))) pseudo_king_moves &= ~(1ULL << t);
  }
  emplaceCaptures(myking, KING, enemy_pawns, enemy_knights, enemy_bishops, enemy_rooks, enemy_queens, pseudo_king_moves & piece_colors[CT^1]);
  emplaceNonCaptures(myking, KING, pseudo_king_moves & ~piece_colors[CT^1]);


  // castles
  if (!(~checkmask)) {
    U64 castle_targets = bbCastles<CT>(pos);
    U64 cst_qs = FILE_C & castle_targets;
    U64 cst_ks = FILE_G & castle_targets;
    if (cst_ks) {
      generated_moves.emplace_back((64 * __builtin_ctzll(cst_ks) + myking) + CASTLES + MOVE_KING);
    }
    if (cst_qs) {
      generated_moves.emplace_back((64 * __builtin_ctzll(cst_qs) + myking) + CASTLES + MOVE_KING);
    }
  }
  // en passant now

  U64 ep_bb = pos.getEpSquare();
  if (ep_bb) {
    U64 ep_ps;
    U64 ep_enemy_pawn;
    if constexpr (CT == COLOR_WHITE) {
      ep_enemy_pawn = ep_bb >> 8;
      ep_ps = piece_colors[CT] & pieces[PAWN-1] & (((ep_bb >> 9) & ~FILE_H) | ((ep_bb >> 7) & ~FILE_A));
    }
    else {
      ep_enemy_pawn = ep_bb << 8;
      ep_ps = piece_colors[CT] & pieces[PAWN-1] & (((ep_bb << 7) & ~FILE_H) | ((ep_bb << 9) & ~FILE_A));      
    }
    for (; ep_ps; ep_ps &= ep_ps-1) {
      int p = __builtin_ctzll(ep_ps);
      if (squareAttackedBy<CTO>(myking, pos, blockers ^ (1ULL << p) ^ ep_bb ^ ep_enemy_pawn)) continue;
      generated_moves.emplace_back((64 * __builtin_ctzll(ep_bb) + p) | ENPASSANT | MOVE_PAWN);
    }
  }
  return 0;
}


}

#endif
