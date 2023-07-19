#include "search.h"
#include <iostream>

namespace Wyvern {

template BoundedEval Search::negamax<COLOR_WHITE>(Position&, int, int, int, bool);
template BoundedEval Search::negamax<COLOR_BLACK>(Position&, int, int, int, bool);

U32 Search::bestmove(Position pos, int depth) {
  enum Color player_turn = pos.getToMove();
  if (player_turn) movegen.generateMoves<COLOR_BLACK>(pos);
  else movegen.generateMoves<COLOR_WHITE>(pos);
  std::vector<U32> moves;
  int best_value = -INT32_MAX;
  std::vector<U32> best_moves;
  U32 m = movegen.popMove(0);
  while (m != MOVE_NONE)
  {
    moves.push_back(m);
    m = movegen.popMove(0);
  }

  std::vector<BoundedEval> b_evals(moves.size());

  // iterative deepening up to depth-2 to get promising move order 
  if (depth >= 3) {
    for (int id_d = 1; id_d < depth-1; id_d++) {
      int t_alpha = -INT32_MAX; // temporary value of alpha for ids
      int i = 0;
      for (U32 move : moves) {
        pos.makeMove(move);
        BoundedEval val;
        if (player_turn == COLOR_WHITE) val = -negamax<COLOR_BLACK>(pos, id_d, -INT32_MAX, -t_alpha, true);
        else val = -negamax<COLOR_WHITE>(pos, id_d, -INT32_MAX, -t_alpha, true);
        pos.unmakeMove();
        b_evals[i]=val; i++;
        if (val.eval > t_alpha) t_alpha = val.eval;
        // no fail, as we want to at least seach all nodes here for move ordering.
      }
      sortMoves(moves, b_evals);
    }
  }
  for (U32 move : moves) {
    std::cout << std::hex << move;
    pos.makeMove(move);
    BoundedEval val(BOUND_EXACT,0);
    if (player_turn)
      val = -negamax<COLOR_WHITE>(pos, depth-1, -INT32_MAX, -best_value, true);
    else
      val = -negamax<COLOR_BLACK>(pos, depth-1, -INT32_MAX, -best_value, true);
    if (val.eval > best_value) {
      best_moves.clear();
      best_moves.push_back(move);
    }
    else if (val.eval == best_value && val.bound == BOUND_EXACT) {
      best_moves.push_back(move);
    }
    best_value = (val.eval >= best_value) ? val.eval : best_value;
    std::string pr_bound = (val.bound == BOUND_EXACT) ? "=="
                         : (val.bound == BOUND_UPPER) ? "<=" : ">=";
    std::cout << " value" << pr_bound << std::dec << val.eval << std::endl;

    pos.unmakeMove();
  }
  if (best_moves.size() == 0) return MOVE_NONE;
  return best_moves[rand64() % best_moves.size()];
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
  for (U32 move = movegen.popMove(0); move != MOVE_NONE; move = movegen.popMove(0)) {
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

void Search::sortMoves(std::vector<U32>& moves, std::vector<BoundedEval>& evals) {
  int lb_count = 0;
  int ub_count = 0;
  int exact_count = 0;
  for (BoundedEval be : evals) {
    if (be.bound == BOUND_EXACT) exact_count++;
    if (be.bound == BOUND_UPPER) ub_count++;
    if (be.bound == BOUND_LOWER) lb_count++;
  }
  for (int i = 0; i < lb_count; i++) {
    for (U64 j = i; j < moves.size(); j++) {
      if (evals[j].bound == BOUND_LOWER) {
        std::swap(evals[i],evals[j]);
        std::swap(moves[i],moves[j]);
        break;
      }
    }
  }
  for (int i = lb_count; i < lb_count+exact_count; i++) {
    int best = INT32_MIN;
    int best_index = -1;
    for (U64 j = i; j < moves.size(); j++) {
      if (evals[j].bound == BOUND_EXACT && evals[j].eval > best) {
        best = evals[j].eval;
        best_index = j;
        break;
      }
    }
    std::swap(evals[i],evals[best_index]);
    std::swap(moves[i],moves[best_index]);
  }
  for (int i = ub_count; i < ub_count; i++) {
    for (U64 j = i; j < moves.size(); j++) {
      if (evals[j].bound == BOUND_UPPER) {
        std::swap(evals[i],evals[j]);
        std::swap(moves[i],moves[j]);
        break;
      }
    }
  }

}

}
