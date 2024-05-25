#ifndef H_GUARD_MOVEGEN
#define H_GUARD_MOVEGEN

#include <bit>

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
                             U64 enemy_pieces, const U64* all_pieces, U64 pp_o, U64 pp_d, std::vector<U32>& move_tgts);
  void emplaceCaptures(int p, enum PieceType pt, U64 enemy_pieces, const U64* all_pieces, U64 targets, std::vector<U32>& move_tgts) {
    for(U64 t = targets & enemy_pieces & all_pieces[0]; t; t &= t - 1) {
        int tp = std::countr_zero(t);
        move_tgts.emplace_back(p + (tp << 6) + CAPTURE_PAWN + ((U32)pt << 20));
    }
    for(U64 t = targets & enemy_pieces & all_pieces[1]; t; t &= t - 1) {
      int tp = std::countr_zero(t);
      move_tgts.emplace_back(p + (tp << 6) + CAPTURE_KNIGHT + ((U32)pt << 20));
    }
    for(U64 t = targets & enemy_pieces & all_pieces[2]; t; t &= t - 1) {
      int tp = std::countr_zero(t);
      move_tgts.emplace_back(p + (tp << 6) + CAPTURE_BISHOP + ((U32)pt << 20));
    }
    for(U64 t = targets & enemy_pieces & all_pieces[3]; t; t &= t - 1) {
      int tp = std::countr_zero(t);
      move_tgts.emplace_back(p + (tp << 6) + CAPTURE_ROOK + ((U32)pt << 20));
    }
    for(U64 t = targets & enemy_pieces & all_pieces[4]; t; t &= t - 1) {
      int tp = std::countr_zero(t);
      move_tgts.emplace_back(p + (tp << 6) + CAPTURE_QUEEN + ((U32)pt << 20));
    }
  }
  void emplaceNonCaptures(int p, enum PieceType pt, U64 targets, std::vector<U32>& move_tgts) {
    for(U64 t = targets; t; t &= t-1) {
      int tp = std::countr_zero(t);
      move_tgts.emplace_back(p + (tp << 6) + ((U32)pt << 20));
    }
  }
  void emplacePromotions(int p, U64 enemy_pieces, const U64* all_pieces,  U64 targets, std::vector<U32>& move_tgts) {
    for (U64 t = targets; t; t&= t-1) {
      int tp = std::countr_zero(t);
      enum MoveCapture cap = NO_CAPTURE;
      U64 tbb = 1ULL << tp;
      if (enemy_pieces & all_pieces[PAWN-1] & tbb) cap = CAPTURE_PAWN;
      if (enemy_pieces & all_pieces[KNIGHT-1] & tbb) cap = CAPTURE_KNIGHT;
      if (enemy_pieces & all_pieces[BISHOP-1] & tbb) cap = CAPTURE_BISHOP;
      if (enemy_pieces & all_pieces[ROOK-1] & tbb) cap = CAPTURE_ROOK;
      if (enemy_pieces & all_pieces[QUEEN-1] & tbb) cap = CAPTURE_QUEEN;
      for (U32 pp = 0; pp <= (3 << 12); pp += 1 << 12) {
        move_tgts.emplace_back(p + (tp << 6) + ((U32)cap) + PROMO + pp + MOVE_PAWN);
      }
    }
  }
public:
  std::shared_ptr<MagicTable> mt;
  template<enum Color CT> U64 squareAttackedBy(int p, const Position& pos, U64 custom_blockers);
  MoveGenerator() = delete;
  MoveGenerator(std::shared_ptr<MagicTable> _mt);
  MoveGenerator(const MoveGenerator&) = delete;
  template<enum Color CT> int generateMoves(Position& pos, bool incl_quiets, std::vector<U32>& move_tgts);
  U64 inCheck(Position& pos);
  ~MoveGenerator() = default;
};

template<enum PieceType PT, enum Color CT>
U64 MoveGenerator::bbPseudoLegalMoves(int p, U64 postmask, U64 bb_blockers) {
  if constexpr (PT == KNIGHT) return mt->knight_table[p] & postmask;
  if constexpr (PT == BISHOP) return mt->bishop_magics[p].compute(bb_blockers) & postmask;
  if constexpr (PT == ROOK) return mt->rook_magics[p].compute(bb_blockers) & postmask;
  if constexpr (PT == KING) return mt->king_table[p] & postmask;
  if constexpr (PT == QUEEN) return (mt->bishop_magics[p].compute(bb_blockers) | mt->rook_magics[p].compute(bb_blockers)) & postmask;
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
  int king = std::countr_zero(pos.getPieces()[KING-1] & pos.getPieceColors()[CT]);
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
U64 MoveGenerator::squareAttackedBy(int p, const Position& pos, U64 custom_blockers) {
  const U64* pcols = pos.getPieceColors();
  const U64* pcs = pos.getPieces();
  U64 blockers = (custom_blockers) ? custom_blockers : (pcols[0] | pcols[1]) & ~(1ULL << p);
  U64 attackers = 0;
  attackers |= pcs[QUEEN-1] & (mt->bishop_magics[p].compute(blockers) | mt->rook_magics[p].compute(blockers));
  attackers |= pcs[ROOK-1] &(mt->rook_magics[p].compute(blockers));
  attackers |= pcs[BISHOP-1] & (mt->bishop_magics[p].compute(blockers));
  attackers |= pcs[KNIGHT-1] & mt->knight_table[p];
  attackers |= pcs[KING-1] & mt->king_table[p];
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
void MoveGenerator::generateStandardMoves(U64 ps, U64 checkmask, U64 blockers, int myking, U64 our_pieces, U64 enemy_pieces, const U64* all_pieces, U64 pp_o, U64 pp_d, std::vector<U32>& move_tgts) {
  for (; ps; ps &= ps-1) {
    int p = std::countr_zero(ps);
    U64 pinmask = makePinmask(p, pp_d, pp_o, blockers, myking, enemy_pieces & (all_pieces[QUEEN-1]|all_pieces[BISHOP-1]), enemy_pieces & (all_pieces[QUEEN-1]|all_pieces[ROOK-1]));
    U64 targets = bbPseudoLegalMoves<PT, CT>(p, checkmask & pinmask & ~our_pieces, blockers);
    if constexpr (PT == PAWN) {
      constexpr U64 promo_rank = RANK_8 >> (56*CT);
      U64 t_promotions = targets & promo_rank;
      U64 t_captures = targets & enemy_pieces & ~promo_rank;
      U64 t_non_caps = targets & ~enemy_pieces & ~promo_rank;
      emplacePromotions(p, enemy_pieces, all_pieces, t_promotions, move_tgts);
      emplaceCaptures(p, PAWN, enemy_pieces, all_pieces, t_captures, move_tgts);
      emplaceNonCaptures(p, PAWN, t_non_caps, move_tgts);
    }
    else if constexpr (PT >= KNIGHT && PT <= QUEEN) {
      int p = std::countr_zero(ps);  
      U64 t_caps = targets & enemy_pieces;
      U64 t_quiets = targets & ~enemy_pieces;
      emplaceCaptures(p, PT, enemy_pieces, all_pieces, t_caps, move_tgts);
      emplaceNonCaptures(p, PT, t_quiets, move_tgts);
    }
  }
}

template<enum Color CT>
int MoveGenerator::generateMoves(Position& pos, bool incl_quiets, std::vector<U32>& move_tgts) {
  move_tgts.reserve(32);
  constexpr enum Color CTO = (enum Color) (CT ^ 1); 
  if constexpr (CT > 1) return 0;
  const U64* pieces = pos.getPieces();
  const U64* piece_colors = pos.getPieceColors();

  U64 blockers = piece_colors[0] | piece_colors[1];
  int myking = std::countr_zero(pieces[KING-1] & piece_colors[CT]); 
  U64 checkmask = 0xFFFFFFFFFFFFFFFFULL; // sneaky all bits set

  U64 checkers = squareAttackedBy<CTO>(myking, pos, 0);
  U64 king_file = files[myking % 8];
  U64 king_rank = ranks[myking / 8];

  if (std::popcount(checkers) > 1) checkmask = 0; // if double check, we can skip all non-king moves
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
          checkmask = mt->rook_magics[std::countr_zero(checkers)].compute(blockers)
                    & mt->rook_magics[myking].compute(blockers);
        }
        else {
          checkmask = mt->bishop_magics[std::countr_zero(checkers)].compute(blockers)
                    & mt->bishop_magics[myking].compute(blockers);
        }
    }
    checkmask |= checkers;
    //std::cout << "in (single) check - here is position and checkmask:\n";
    //pos.printPretty();
    //printbb(checkmask);
  }
  // checkmask now holds all possible target squares to block or capture a checking piece if applicable

  U64 v_targets = checkmask & ((incl_quiets) ? 0xFFFFFFFFFFFFFFFFULL : piece_colors[CT^1]);

  //printbb(checkmask);
  if (checkmask) {
    U64 pp_d = mt->bishop_magics[myking].compute(blockers);
    U64 pp_o = mt->rook_magics[myking].compute(blockers);

    U64 promo_rank = checkmask & ((CT) ? RANK_1 : RANK_8);
    // move ordering:
    // pxq, pxr, px
    generateStandardMoves<PAWN,CT>(piece_colors[CT] & pieces[PAWN-1], v_targets | promo_rank, blockers, myking,
                                   piece_colors[CT], piece_colors[CTO], pieces, pp_o, pp_d, move_tgts);
    generateStandardMoves<KNIGHT,CT>(piece_colors[CT] & pieces[KNIGHT-1], v_targets, blockers, myking,
                                   piece_colors[CT], piece_colors[CTO], pieces, pp_o, pp_d, move_tgts);
    generateStandardMoves<BISHOP,CT>(piece_colors[CT] & pieces[BISHOP-1], v_targets, blockers, myking,
                                   piece_colors[CT], piece_colors[CTO], pieces, pp_o, pp_d, move_tgts);
    generateStandardMoves<ROOK,CT>(piece_colors[CT] & pieces[ROOK-1], v_targets, blockers, myking,
                                   piece_colors[CT], piece_colors[CTO], pieces, pp_o, pp_d, move_tgts);
    generateStandardMoves<QUEEN,CT>(piece_colors[CT] & pieces[QUEEN-1], v_targets, blockers, myking,
                                   piece_colors[CT], piece_colors[CTO], pieces, pp_o, pp_d, move_tgts);

  }
  // king moves
  U64 pseudo_king_moves = bbPseudoLegalMoves<KING, CT>(myking, ~piece_colors[CT], blockers);
  for (U64 tt = pseudo_king_moves; tt; tt &= tt-1) {
    int t = std::countr_zero(tt);
    if (squareAttackedBy<CTO>(t, pos, blockers ^ (1ULL << myking))) pseudo_king_moves &= ~(1ULL << t);
  }
  emplaceCaptures(myking, KING, piece_colors[CTO], pieces, pseudo_king_moves & piece_colors[CT^1], move_tgts);
  if (incl_quiets) emplaceNonCaptures(myking, KING, pseudo_king_moves & ~piece_colors[CT^1], move_tgts);


  // castles
  if (!(~checkmask) && incl_quiets) {
    U64 castle_targets = bbCastles<CT>(pos);
    U64 cst_qs = FILE_C & castle_targets;
    U64 cst_ks = FILE_G & castle_targets;
    if (cst_ks) {
      move_tgts.emplace_back((64 * std::countr_zero(cst_ks) + myking) + CASTLES + MOVE_KING);
    }
    if (cst_qs) {
      move_tgts.emplace_back((64 * std::countr_zero(cst_qs) + myking) + CASTLES + MOVE_KING);
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
      int p = std::countr_zero(ep_ps);
      if (squareAttackedBy<CTO>(myking, pos, blockers ^ (1ULL << p) ^ ep_bb ^ ep_enemy_pawn)) continue;
      move_tgts.emplace_back((64 * std::countr_zero(ep_bb) + p) | ENPASSANT | MOVE_PAWN);
    }
  }
  return 0;
}


}

#endif
