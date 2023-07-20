#include "search.h"
#include <iostream>

namespace Wyvern {

template BoundedEval Search::negamax<COLOR_WHITE>(Position&, int, int, int, bool);
template BoundedEval Search::negamax<COLOR_BLACK>(Position&, int, int, int, bool);

U32 Search::bestmove(Position pos, double t_limit) {
  init_time = time(nullptr);
  time_limit = t_limit;
  enum Color player_turn = pos.getToMove();
  if (player_turn) movegen.generateMoves<COLOR_BLACK>(pos, true);
  else movegen.generateMoves<COLOR_WHITE>(pos, true);
  std::vector<U32> moves;
  U32 m = movegen.popMove(0);
  while (m != MOVE_NONE)
  {
    moves.push_back(m);
    m = movegen.popMove(0);
  }
  if (moves.size() == 0) return MOVE_NONE;
  if (moves.size() == 1) return moves.back();

  std::vector<BoundedEval> b_evals(moves.size());
  U32 best_move = MOVE_NONE;
  int best_eval = -INT32_MAX;

  for (int id_d = 1; difftime(time(nullptr), init_time) < time_limit; id_d++) {
    int t_alpha = -INT32_MAX; // temporary value of alpha for ids
    int i = 0;
    for (U32 move : moves) {
      pos.makeMove(move);
      BoundedEval val;
      if (player_turn == COLOR_WHITE) val = -negamax<COLOR_BLACK>(pos, id_d, -INT32_MAX, -t_alpha, true);
      else val = -negamax<COLOR_WHITE>(pos, id_d, -INT32_MAX, -t_alpha, true);
      pos.unmakeMove();
      if (difftime(time(nullptr), init_time) >= time_limit) return best_move;
      b_evals[i]=val; i++;
      if (val.eval > best_eval && val.bound != BOUND_LOWER) {
        best_eval = val.eval; best_move = move;
      } 
      if (val.eval > t_alpha) t_alpha = val.eval;
      // no fail, as we want to at least seach all nodes here for move ordering.
    }
    sortMoves(moves, b_evals);
    BoundedEval ref_beval = bestEvalInVector(b_evals);
    for (size_t i = 0; i < b_evals.size(); ++i) {
      if (b_evals[i] == ref_beval) best_move = moves[i];
    }
  }

  return (best_move);

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
  if (ct == COLOR_WHITE) movegen.generateMoves<COLOR_WHITE>(pos, true);
  if (ct == COLOR_BLACK) movegen.generateMoves<COLOR_BLACK>(pos, true);
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
}

bool Search::checkThreeReps(Position& pos) {
  auto begin = pos.positionHistoryIteratorBegin();
  auto end = pos.positionHistoryIteratorEnd();
  int hmc = pos.getHMC();
  int count = 0;
  U64 z = pos.getZobrist();
  for (auto iter = begin; iter != end && (begin - iter) < hmc; ++iter) {
    if (iter[0] == z) count++;
    if (count == 2) return true;
  }
  return false;
}


BoundedEval Search::bestEvalInVector(std::vector<BoundedEval>& b_evals) {
  BoundedEval best = BoundedEval(BOUND_EXACT, -INT32_MAX);
  for (BoundedEval b : b_evals) {
    if (b.eval > best.eval && (b.bound == BOUND_EXACT || b.bound == BOUND_LOWER)) best = b;
  }
  return best;
}

Search::Search():
mt(std::make_shared<MagicTable>()),
evaluator(mt),
movegen(mt)
{
}

}
