#ifndef H_GUARD_SEARCH
#define H_GUARD_SEARCH

#include "position.h"
#include "types.h"
#include "movegen.h"
#include "evaluate.h"

namespace Wyvern {

class Search {
private:
  Evaluator evaluator;
  MoveGenerator movegen;
  void sortMoves(std::vector<U32>& moves, std::vector<BoundedEval>& evals);
public:
  Search() = default;
  U32 bestmove(Position pos, int depth);
  template<enum Color CT>
  BoundedEval negamax(Position& pos, int depth, int alpha, int beta,  bool iter_deep);
  ~Search() = default;
  U64 perft(Position &pos, int depth, int max_depth, int* n_capts, int* n_enpass, int* n_promo, int* n_castles, int* checks);
};



template<enum Color CT>
BoundedEval Search::negamax(Position& pos, int depth, int alpha, int beta, bool iter_deep) {

  constexpr enum Color CTO = (enum Color) (CT^1);
  // if terminal return draw, win, loss as appropriate
  // todo if
  movegen.generateMoves<CT>(pos);
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
  if (iter_deep && moves.size() >= 6 && depth >= 3) {
    for (int id_d = 1; id_d < depth-1; id_d++) {
      int t_alpha = alpha; // temporary value of alpha for ids
      int i = 0;
      for (U32 move : moves) {
        pos.makeMove(move);
        int extension= 0;
        if ((move & YES_CAPTURE) || movegen.inCheck(pos)) extension = 1;
        BoundedEval val = -negamax<CTO>(pos, id_d + extension, -beta, -t_alpha, iter_deep);
        pos.unmakeMove();
        b_evals[i++]=val;
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
    int extension= 0;
    if ((move & YES_CAPTURE) || movegen.inCheck(pos)) extension = 1;
    BoundedEval val = -negamax<CTO>(pos, depth - 1 + extension, -beta, -alpha, iter_deep);
    pos.unmakeMove();
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
