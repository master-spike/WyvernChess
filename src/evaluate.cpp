
#include "evaluate.h"

namespace Wyvern {

constexpr int place_value_pawn[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10,-20,-20, 10, 10,  5,
    5, -5,-10,  0,  0,-10, -5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5,  5, 10, 25, 25, 10,  5,  5,
   10, 10, 20, 30, 30, 20, 10, 10,
   50, 50, 50, 50, 50, 50, 50, 50,
    0,  0,  0,  0,  0,  0,  0,  0
};

constexpr int place_value_knight[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

constexpr int place_value_bishop[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

constexpr int place_value_rook[64] = {
    0,  0,  0,  5,  5,  0,  0,  0,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
    5, 10, 10, 10, 10, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

constexpr int place_value_queen[64] = {
   -20,-10,-10, -5, -5,-10,-10,-20,
   -10,  0,  5,  0,  0,  0,  0,-10,
   -10,  5,  5,  5,  5,  5,  0,-10,
     0,  0,  5,  5,  5,  5,  0, -5,
    -5,  0,  5,  5,  5,  5,  0, -5,
   -10,  0,  5,  5,  5,  5,  0,-10,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -20,-10,-10, -5, -5,-10,-10,-20
};

constexpr int king_middle_game[64] = {
 20, 30, 10,  0,  0, 10, 30, 20,
 20, 20,  0,  0,  0,  0, 20, 20, 
-10,-20,-20,-20,-20,-20,-20,-10,
-20,-30,-30,-40,-40,-30,-30,-20,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30
};

constexpr int king_end_game[64] = {
-50,-30,-30,-30,-30,-30,-30,-50,
-30,-30,  0,  0,  0,  0,-30,-30,
-30,-10, 20, 30, 30, 20,-10,-30,
-30,-10, 30, 40, 40, 30,-10,-30,
-30,-10, 30, 40, 40, 30,-10,-30,
-30,-10, 20, 30, 30, 20,-10,-30,
-30,-20,-10,  0,  0,-10,-20,-30,
-50,-40,-30,-20,-20,-30,-40,-50
};

constexpr int passed_pawn_value = 50;

constexpr int queen_mobility_factor = 3;
constexpr int bishop_mobility_factor = 4;
constexpr int rook_mobility_factor = 4;

int psqvTableLookup(enum Color ct, int p, const int* table) {
  if (ct == COLOR_WHITE) return table[p];
  else return table[(p & 7) + 56 - (p&56)];
}

int Evaluator::evalMaterialOnly(Position& pos){
  enum Color player = pos.getToMove();
  enum Color opponent = (player == COLOR_BLACK) ? COLOR_WHITE : COLOR_BLACK;
  U64* pcols = pos.getPieceColors();
  U64* pcs = pos.getPieces();
  int total = 0;
  for (int i = 0; i < 5; ++i) {
    total += pvals[i] * __builtin_popcountll(pcols[player] & pcs[i]);
    total -= pvals[i] * __builtin_popcountll(pcols[opponent] & pcs[i]);
  }
  //total += (int) (rand() % 101) - 50;
  return total;
}

int Evaluator::totalMaterial(Position& pos) {
  U64* pcs = pos.getPieces();
  int total_material = 0;
  total_material += __builtin_popcountll(pcs[PAWN-1]);
  total_material += 3*__builtin_popcountll(pcs[KNIGHT-1] | pcs[BISHOP-1]);
  total_material += 5*__builtin_popcountll(pcs[ROOK-1]);
  total_material += 9*__builtin_popcountll(pcs[QUEEN-1]);
  return total_material;
}

int Evaluator::evalPositional(Position& pos) {
  enum Color player = pos.getToMove();
  enum Color opponent = (player == COLOR_BLACK) ? COLOR_WHITE : COLOR_BLACK;
  U64* pcols = pos.getPieceColors();
  U64* pcs = pos.getPieces();
  int total = 0;
  int total_material = totalMaterial(pos);
  total_material += __builtin_popcountll(pcs[PAWN-1]);
  total_material += 3*__builtin_popcountll(pcs[KNIGHT-1] | pcs[BISHOP-1]);
  total_material += 5*__builtin_popcountll(pcs[ROOK-1]);
  total_material += 9*__builtin_popcountll(pcs[QUEEN-1]);
  
  constexpr int eg_mg_diff = midgame_material_limit - endgame_material_limit;

  U64 bb_blockers = (pcols[opponent] | pcols[player]);
  U64 pawns_black = pcols[1] & pcs[PAWN-1];
  U64 pawns_white = pcols[0] & pcs[PAWN-1];
  U64 our_pawn_cs = (player) ? (pawns_black >> 7 & ~FILE_A) | (pawns_black >> 9 & ~FILE_H)
                             : (pawns_white << 7 & ~FILE_H) | (pawns_white << 9 & ~FILE_A);
  U64 opp_pawn_cs = (opponent) ? (pawns_black >> 7 & ~FILE_A) | (pawns_black >> 9 & ~FILE_H)
                               : (pawns_white << 7 & ~FILE_H) | (pawns_white << 9 & ~FILE_A);
  
  int endgame_interp = (total_material <= endgame_material_limit) ? 0 : 
                       (total_material >= midgame_material_limit) ? eg_mg_diff
                       : (eg_mg_diff * (total_material - endgame_material_limit)) / eg_mg_diff;
  
  for (U64 pawns = pcs[PAWN-1] & pcols[player] ; pawns ; pawns&=pawns-1) {
    int p = __builtin_ctzll(pawns);
    total += psqvTableLookup(player, p, place_value_pawn);
    if (!(mt->passed_pawns[p + 64*player] & pcs[PAWN-1] & pcols[opponent])) total += passed_pawn_value;
  }
  for (U64 pawns = pcs[PAWN-1] & pcols[opponent] ; pawns ; pawns&=pawns-1) {
    int p = __builtin_ctzll(pawns);
    total -= psqvTableLookup(opponent, p, place_value_pawn);
    if (!(mt->passed_pawns[p + 64*opponent] & pcs[PAWN-1] & pcols[player])) total -= passed_pawn_value;

  }
  
  for (U64 knights = pcs[KNIGHT-1] & pcols[player] ; knights ; knights&=knights-1) {
    total += psqvTableLookup(player,__builtin_ctzll(knights), place_value_knight);
  }
  for (U64 knights = pcs[KNIGHT-1] & pcols[opponent] ; knights ; knights&=knights-1) {
    total -= psqvTableLookup(opponent,__builtin_ctzll(knights), place_value_knight);
  }

  for (U64 bishops = pcs[BISHOP-1] & pcols[player] ; bishops ; bishops&=bishops-1) {
    int p = __builtin_ctzll(bishops);
    total += psqvTableLookup(player,p, place_value_bishop);
    U64 targets = mt->bishop_magics[p].compute(bb_blockers) & ~bb_blockers & ~opp_pawn_cs;
    total += (__builtin_popcountll(targets) * bishop_mobility_factor * endgame_interp) / eg_mg_diff;
  }
  for (U64 bishops = pcs[BISHOP-1] & pcols[opponent] ; bishops ; bishops&=bishops-1) {
    int p = __builtin_ctzll(bishops);
    total -= psqvTableLookup(opponent,p, place_value_bishop);
    U64 targets = mt->bishop_magics[p].compute(bb_blockers) & ~bb_blockers & ~our_pawn_cs;
    total -= (__builtin_popcountll(targets) * bishop_mobility_factor * endgame_interp) / eg_mg_diff;
  }

  for (U64 rooks = pcs[ROOK-1] & pcols[player] ; rooks ; rooks&=rooks-1) {
    int p = __builtin_ctzll(rooks);
    total += psqvTableLookup(player,p, place_value_rook);
    U64 targets = mt->rook_magics[p].compute(bb_blockers) & ~bb_blockers & ~opp_pawn_cs;
    total += (__builtin_popcountll(targets) * rook_mobility_factor * endgame_interp) / eg_mg_diff;
  }
  for (U64 rooks = pcs[ROOK-1] & pcols[opponent] ; rooks ; rooks&=rooks-1) {
    int p = __builtin_ctzll(rooks);
    total -= psqvTableLookup(opponent,p, place_value_rook);
    U64 targets = mt->rook_magics[p].compute(bb_blockers) & ~bb_blockers & ~our_pawn_cs;
    total -= (__builtin_popcountll(targets) * rook_mobility_factor * endgame_interp) / eg_mg_diff;
  }

  for (U64 queens = pcs[QUEEN-1] & pcols[player] ; queens ; queens&=queens-1) {
    int p = __builtin_ctzll(queens);
    total += psqvTableLookup(player,p, place_value_rook);
    U64 targets = (mt->rook_magics[p].compute(bb_blockers) |
                   mt->bishop_magics[p].compute(bb_blockers)) & ~bb_blockers & ~opp_pawn_cs;
    total += (__builtin_popcountll(targets) * queen_mobility_factor * endgame_interp) / eg_mg_diff;
  }
  for (U64 queens = pcs[QUEEN-1] & pcols[opponent] ; queens ; queens&=queens-1) {
    int p = __builtin_ctzll(queens);
    total -= psqvTableLookup(opponent,p, place_value_rook);
    U64 targets = (mt->rook_magics[p].compute(bb_blockers) |
                   mt->bishop_magics[p].compute(bb_blockers)) & ~bb_blockers & ~our_pawn_cs;
    total -= (__builtin_popcountll(targets) * queen_mobility_factor * endgame_interp) / eg_mg_diff;
  }

  int myking = __builtin_ctzll(pcs[KING-1] & pcols[player]);
  int kvals = endgame_interp * psqvTableLookup(player, myking, king_middle_game);
  kvals += (eg_mg_diff - endgame_interp) * psqvTableLookup(player, myking, king_end_game);
  int opking = __builtin_ctzll(pcs[KING-1] & pcols[opponent]);
  kvals -= endgame_interp * psqvTableLookup(opponent, opking, king_middle_game);
  kvals -= (eg_mg_diff - endgame_interp) * psqvTableLookup(opponent, opking, king_end_game);
  total += kvals / eg_mg_diff;
  total += evalMaterialOnly(pos);

  total += 30; // 30 centipawns for side to move

  return total;
}



Evaluator::Evaluator(std::shared_ptr<MagicTable> _mt) {
  mt = std::shared_ptr<MagicTable>(_mt);
}


U64 see_lvp(U64 attadef, U64 side_pcs, U64* pieceBB, enum PieceType& aPiece) {
  for (int p = 0; p < 6; p++) {
    U64 set = side_pcs & attadef & pieceBB[p];
    if (set) {
      aPiece = (enum PieceType) (p+1);
      return set & (set-1);
    }
  }
  return 0;
}

// based on https://www.chessprogramming.org/SEE_-_The_Swap_Algorithm
int Evaluator::see(Position& pos, enum PieceType piece, enum PieceType target, int frsq, int tosq, int side)
{
  enum PieceType aPiece = piece;
  U64* pcs = pos.getPieces();
  U64* pcols = pos.getPieceColors();
  U64 blockers = pcs[0] | pcs[1] | pcs[2] | pcs[3] | pcs[4] | pcs[5];
  int gain[32];
  U64 fromset = 1ULL << frsq;
  U64 may_xray_diag = pcs[0] | pcs[2] |  pcs[4]; // pawns, bishops, queens
  U64 may_xray_orth = pcs[3] | pcs[4]; // rooks, queens
  U64 attadef = (mt->bishop_magics[tosq].compute(blockers) & (pcs[2] | pcs[4]))
              | (mt->rook_magics[tosq].compute(blockers) & (pcs[3] | pcs[4]))
              | (mt->knight_table[tosq] & pcs[1]) | (mt->king_table[tosq] & pcs[5])
              | (((pcs[0] & pcols[1]) >> 7 & ~FILE_A) | ((pcs[0] & pcols[1]) >> 9 & ~FILE_H))
              | (((pcs[0] & pcols[0]) << 7 & ~FILE_H) | ((pcs[0] & pcols[0]) << 9 & ~FILE_A));
  
  U64 ad_xray = (mt->bishop_magics[tosq].compute(blockers & ~attadef) & (pcs[2] | pcs[4]))
              | (mt->rook_magics[tosq].compute(blockers & ~attadef) & (pcs[3] | pcs[4]));
  ad_xray &= ~attadef;

  int d = 0;
  gain[d] = pvals[(int)target-1];
  while(fromset) {
    d++;
    side ^= 1;
    gain[d] = pvals[aPiece - 1] - gain[d-1];
    if (MAX_INT(gain[d], -gain[d-1]) < 0) break;
    attadef ^= fromset;
    blockers ^= fromset;
    if (fromset & may_xray_orth)
      attadef |= mt->rook_magics[tosq].compute(blockers) & (pcs[3] | pcs[4]);
    if (fromset & may_xray_diag)
      attadef |= mt->bishop_magics[tosq].compute(blockers) & (pcs[2] | pcs[4]);
    fromset = see_lvp(attadef, pcols[side], pcs, aPiece);
  }
  while(--d) {
    gain[d-1] = -MAX_INT(-gain[d-1], gain[d]);
  }

  return gain[0];
}

}
