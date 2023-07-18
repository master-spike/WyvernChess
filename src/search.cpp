#include "search.h"
#include <iostream>

namespace Wyvern {

template<enum Color CT>
int Search::negamax(Position& pos, int depth, int alpha, int beta) {
  // if terminal return draw, win, loss as appropriate
  // todo if
  movegen.generateMoves<CT>(pos);
  U16 move = movegen.popMove();
  if (move == MOVE_NONE) {
    //terminal, return mate or stalemate
    return 0;
  }
  if (pos.getHMC() >= 50) return 0;
  if (depth == 0) {
    return 0; // static
  } 
  int best_val = alpha;
  for (;move != MOVE_NONE; move = movegen.popMove()) {
    pos.makeMove(move);
    int val = negamax<CT^1>(pos, depth - 1, -beta, -alpha);
    alpha = (val > alpha) ? val : alpha;
    if (alpha >= beta) break;
    best_val = (val > best_val) ? val : best_val;
    pos.unmakeMove();
  }
  return best_val;
}


U64 Search::perft(Position& pos, int depth, int max_depth, int* n_capts, int* n_enpass, int* n_promo, int* n_castles, int* checks) {

  if (pos.checkValidity()) {
    pos.printPretty();
    for (U32 move : pos.move_history) {
      std::cout << std::hex << (move>>12) << "|";
      printSq(move & 63); printSq((move>>6)&63);
      std::cout << " -> ";
    }
    std::cout << "\n";
    std::cin.get();
  }
  U64* pcols = pos.getPieceColors();
  U64* pcs = pos.getPieces();
  enum Color ct = pos.getToMove();
  int black_kp = __builtin_ctzll(pcs[KING-1] & pcols[1]);
  int white_kp = __builtin_ctzll(pcs[KING-1] & pcols[0]);
  //if (__builtin_popcountll(pcs[KING-1] & pcols[0]) != 1) return 1;
  //if (__builtin_popcountll(pcs[KING-1] & pcols[1]) != 1) return 1;
  U64 bk_attack = movegen.squareAttackedBy<COLOR_WHITE>(black_kp, pos, 0);
  U64 wk_attack = movegen.squareAttackedBy<COLOR_BLACK>(white_kp, pos, 0);

  U64 checkers = (ct) ? bk_attack : wk_attack;
  if (depth == 0) {
    if (checkers) (*checks)++;
    return 1;
  }
  int sum = 0;
  if (ct == COLOR_WHITE) movegen.generateMoves<COLOR_WHITE>(pos);
  if (ct == COLOR_BLACK) movegen.generateMoves<COLOR_BLACK>(pos);
  std::vector<U32> moves;
  for (U32 move = movegen.popMove(); move != MOVE_NONE; move = movegen.popMove()) {
    moves.push_back(move);
  }
  for (U32 move : moves) {
    
    /*
    for (int indent = 0; indent < max_depth - depth; indent++) {
      std::cout << " - ";
    }
    printSq(move & 63); printSq((move >> 6) & 63); std::cout << std::endl;
    */
    if (depth == 1) {
      if (move & YES_CAPTURE) ++(*n_capts);
      if ((move & MOVE_SPECIAL) == ENPASSANT) {++(*n_enpass); ++(*n_capts);}
      if ((move & MOVE_SPECIAL) == PROMO) ++(*n_promo);
      if ((move & MOVE_SPECIAL) == CASTLES) ++(*n_castles);
    }
    pos.makeMove(move);
    sum += perft(pos, depth-1, max_depth, n_capts, n_enpass, n_promo, n_castles, checks);
    pos.unmakeMove();

  }
  return sum;
}

}
