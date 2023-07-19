#ifndef H_GUARD_SEARCH
#define H_GUARD_SEARCH

#include "position.h"
#include "types.h"
#include "movegen.h"
#include "evaluate.h"
#include <ctime>

namespace Wyvern {

class Search {
private:
  Evaluator evaluator;
  MoveGenerator movegen;
  void sortMoves(std::vector<U32>& moves, std::vector<BoundedEval>& evals);
  template<enum Color CT>
  BoundedEval quiesce(Position pos, int alpha, int beta);
  time_t init_time;
  time_t time_limit;
  BoundedEval bestEvalInVector(std::vector<BoundedEval>& b_evals);
public:
  Search() = default;
  U32 bestmove(Position pos, double t_limit);
  template<enum Color CT>
  BoundedEval negamax(Position& pos, int depth, int alpha, int beta,  bool iter_deep);
  ~Search() = default;
  U64 perft(Position &pos, int depth, int max_depth, int* n_capts, int* n_enpass, int* n_promo, int* n_castles, int* checks);
};

template<enum Color CT> 
BoundedEval Search::quiesce(Position pos, int alpha, int beta) {
  constexpr enum Color CTO = (enum Color) (CT^1);
  U64 checks = movegen.inCheck(pos);
  
  if (checks) movegen.generateMoves<CT>(pos, true);
  else movegen.generateMoves<CT>(pos, false);
  
  // if in check any move that avoids mate is good
  int stand_pat = (checks) ? -INT32_MAX : evaluator.evalPositional(pos);
  std::vector<U32> moves = std::vector<U32>(0,0);
  U32 m = movegen.popMove(0);
  while (m != MOVE_NONE) {
    moves.push_back(m);
    m = movegen.popMove(0);
  }
  if (moves.size() == 0) {
    if (checks) return BoundedEval(BOUND_EXACT, -INT32_MAX);
    movegen.generateMoves<CT>(pos, true); // generate more moves to check for 
    if (movegen.popMove(0) == MOVE_NULL) return BoundedEval(BOUND_EXACT, 0); // stalemate

    // not stalemate, no captures, return stand pat
    return BoundedEval(BOUND_EXACT, stand_pat);
  }

  alpha = (alpha > stand_pat) ? alpha : stand_pat; //baseline score
  enum Bound bound = BOUND_EXACT;
  // now we do captures.
  for (U32 move : moves) {
    pos.makeMove(move);
    BoundedEval val = -quiesce<CTO>(pos, -beta, -alpha);
    // prefer further mates / closer mates
    if (val.eval <= 40-INT32_MAX) val.eval++;
    if (val.eval >= INT32_MAX-40) val.eval--;
    pos.unmakeMove();
    alpha = (val.eval > alpha) ? val.eval : alpha;
    stand_pat = (val.eval > stand_pat) ? val.eval : stand_pat;
    bound = (val.eval > stand_pat) ? val.bound : bound;
    if (alpha >= beta) {
      //std::cout << "cutoff" << std::endl;
      bound = BOUND_LOWER; break;
    }
  }

}
template<enum Color CT>
BoundedEval Search::negamax(Position& pos, int depth, int alpha, int beta, bool iter_deep) {

  constexpr enum Color CTO = (enum Color) (CT^1);
  // if terminal return draw, win, loss as appropriate
  // todo if
  movegen.generateMoves<CT>(pos, true);
  std::vector<U32> moves = std::vector<U32>(0,0);
  U32 m = movegen.popMove(0);
  while (m != MOVE_NONE) {
    moves.push_back(m);
    m = movegen.popMove(0);
  }
  if (moves.size() == 0) {
    if (movegen.inCheck(pos)) return BoundedEval(BOUND_EXACT,-INT32_MAX);
    return BoundedEval(BOUND_EXACT, 0);
  }
  if (pos.getHMC() >= 50) return BoundedEval(BOUND_EXACT, 0);
  if (depth == 0) {
    return BoundedEval(BOUND_EXACT,evaluator.evalPositional(pos));
  }
  std::vector<BoundedEval> b_evals(moves.size());

  // iterative deepening up to depth-2 to get promising move order 
  if (iter_deep && moves.size() >= 6 && depth >= 2) {
    for (int id_d = 1; id_d < depth-1; id_d++) {
      int t_alpha = alpha; // temporary value of alpha for ids
      int i = 0;
      for (U32 move : moves) {
        pos.makeMove(move);
        int extension= 0;
        if ((move & YES_CAPTURE) || movegen.inCheck(pos)) extension = 1;
        BoundedEval val = -negamax<CTO>(pos, id_d + extension, -beta, -t_alpha, iter_deep);
        if (val.eval <= 40-INT32_MAX) val.eval++;
        if (val.eval >= INT32_MAX-40) val.eval--;
        pos.unmakeMove();
        b_evals[i++]=val;
        if (difftime(time(nullptr), init_time) >= time_limit) {
          BoundedEval r_eval = bestEvalInVector(b_evals);
          r_eval.bound = BOUND_LOWER; return r_eval;
        }
        if (val.eval > t_alpha) t_alpha = val.eval;
        // no fail, as we want to at least seach all nodes here for move ordering.
      }
      sortMoves(moves, b_evals);
    }
  }
  int best_val = -INT32_MAX;
  enum Bound bound = BOUND_EXACT;
  for (U32 move : moves) {
    pos.makeMove(move);
    int i = 0;
    int extension= 0;
    if ((move & YES_CAPTURE) || movegen.inCheck(pos)) extension = 1;
    BoundedEval val = -negamax<CTO>(pos, depth - 1 + extension, -beta, -alpha, iter_deep);
    // to prefer further mates / closer mates
    if (val.eval <= 40-INT32_MAX) val.eval++;
    if (val.eval >= INT32_MAX-40) val.eval--;
    pos.unmakeMove();
    b_evals[i++]=val;
    if (difftime(time(nullptr), init_time) >= time_limit) {
      BoundedEval r_eval = bestEvalInVector(b_evals);
      r_eval.bound = BOUND_LOWER; return r_eval;
    }
    alpha = (val.eval > alpha) ? val.eval : alpha;
    best_val = (val.eval > best_val) ? val.eval : best_val;
    bound = (val.eval > best_val) ? val.bound : bound;
    if (alpha >= beta) {
      //std::cout << "cutoff" << std::endl;
      bound = BOUND_LOWER; break;
    }
  }
  return BoundedEval(bound, best_val);
}

}

#endif
