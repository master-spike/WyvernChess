#ifndef H_GUARD_SEARCH
#define H_GUARD_SEARCH

#include "position.h"
#include "types.h"
#include "movegen.h"
#include "evaluate.h"
#include "transposition.h"
#include <ctime>

namespace Wyvern {

class Search {
private:
  std::shared_ptr<MagicTable> mt;
  Evaluator evaluator;
  MoveGenerator movegen;
  void sortMoves(std::vector<U32>& moves, std::vector<BoundedEval>& evals);
  template<enum Color CT>
  BoundedEval quiesce(Position pos, int alpha, int beta);
  time_t init_time;
  time_t time_limit;
  BoundedEval bestEvalInVector(std::vector<BoundedEval>& b_evals);
  bool checkThreeReps(Position& pos);
  U64 node_count;
  U64 node_count_qs;
  U64 table_hits;
  int current_depth;
  int max_depth;
  TranspositionTable ttable;
  void printStats();
public:
  Search();
  U32 bestmove(Position pos, double t_limit);
  template<enum Color CT>
  BoundedEval negamax(Position& pos, int depth, int alpha, int beta,  bool do_quiesce);
  ~Search() = default;
  U64 perft(Position &pos, int depth, int max_depth, int* n_capts, int* n_enpass, int* n_promo, int* n_castles, int* checks);
};

template<enum Color CT> 
BoundedEval Search::quiesce(Position pos, int alpha, int beta) {
  if (current_depth > max_depth) max_depth = current_depth;
  ++node_count;
  ++node_count_qs;
  constexpr enum Color CTO = (enum Color) (CT^1);
  U64 checks = movegen.inCheck(pos);
  
  std::vector<U32> moves;
  if (checks) movegen.generateMoves<CT>(pos, true, &moves);
  else movegen.generateMoves<CT>(pos, false, &moves);
  
  // if in check any move that avoids mate is good
  int stand_pat = (checks) ? -INT32_MAX : evaluator.evalPositional(pos);

  if (moves.size() == 0) {
    std::vector<U32> temp;
    movegen.generateMoves<CT>(pos, true, &temp); // generate more moves to check for mate/stalemate
    if (checks && temp.size() == 0) return BoundedEval(BOUND_EXACT, -INT32_MAX);
    if (temp.size() == 0) return BoundedEval(BOUND_EXACT, 0); // stalemate
    // not stalemate, no captures, return stand pat
  }
  if (pos.getHMC() >= 50) return BoundedEval(BOUND_EXACT, 0);
  if (checkThreeReps(pos)) return BoundedEval(BOUND_EXACT, 0);
  if (moves.size() == 0) return BoundedEval(BOUND_EXACT, stand_pat);
  alpha = (alpha > stand_pat) ? alpha : stand_pat; //baseline score
  enum Bound bound = BOUND_EXACT;
  
  BoundedEval table_lookup = ttable.lookup(pos.getZobrist(), 0);
  if (!(table_lookup.bound & BOUND_INVALID)) {
    ++table_hits;
    return table_lookup;
  }
  // now we do captures.
  for (U32 move : moves) {
    pos.makeMove(move);
    ++current_depth;
    BoundedEval val = -quiesce<CTO>(pos, -beta, -alpha);
    pos.unmakeMove();
    --current_depth;
    // prefer further mates / closer mates
    if (val.eval <= 40-INT32_MAX) val.eval++;
    if (val.eval >= INT32_MAX-40) val.eval--;

    alpha = (val.eval > alpha) ? val.eval : alpha;
    stand_pat = (val.eval > stand_pat) ? val.eval : stand_pat;
    bound = (val.eval > stand_pat) ? val.bound : bound;
    if (alpha >= beta) {
      //std::cout << "cutoff" << std::endl;
      bound = BOUND_LOWER; break;
    }
  }
  ttable.insert(pos.getZobrist(),BoundedEval(bound, stand_pat),0);

  return BoundedEval(bound, stand_pat);

}
template<enum Color CT>
BoundedEval Search::negamax(Position& pos, int depth, int alpha, int beta, bool do_quiesce) {

  if (current_depth > max_depth) max_depth = current_depth;
  ++node_count;
  constexpr enum Color CTO = (enum Color) (CT^1);
  // if terminal return draw, win, loss as appropriate
  // todo if
  std::vector<U32> moves;
  movegen.generateMoves<CT>(pos, true, &moves);
  if (moves.size() == 0) {
    if (movegen.inCheck(pos)) return BoundedEval(BOUND_EXACT,-INT32_MAX);
    return BoundedEval(BOUND_EXACT, 0);
  }
  if (pos.getHMC() >= 50) return BoundedEval(BOUND_EXACT, 0);
  if (checkThreeReps(pos)) return BoundedEval(BOUND_EXACT, 0);
  if (depth == 0) {
    if (!do_quiesce) return BoundedEval(BOUND_EXACT, evaluator.evalPositional(pos));
    return quiesce<CT>(pos, alpha, beta);
  }
  std::vector<BoundedEval> b_evals(moves.size());

  BoundedEval best_so_far = BoundedEval(BOUND_EXACT, -INT32_MAX);
  int bsf_depth = -1;

  // iterative deepening up to depth-2 to get promising move order 
  for (int id_d = 0; id_d < depth; id_d++) {
    int t_alpha = alpha; // temporary value of alpha for ids
    int i = 0;
    for (U32 move : moves) {
      pos.makeMove(move);
      current_depth++;
      int extension= 0;
      if ((move & YES_CAPTURE) || movegen.inCheck(pos)) extension = 1;

      //passed pawn push
      U64 enemy_pawns = pos.getPieceColors()[CT^1] & pos.getPieces()[PAWN-1];
      if ((move >> 20 & 7) == PAWN && !(enemy_pawns & mt->passed_pawns[64*CT + (move&63)])) extension = 1;
      
      bool lmr = false;
      // do not do lmr on captures, check evasions, shallow depth searches, early moves.
      if (!extension &&  !((move & CASTLES) == PROMO) && i >= 4 && id_d > 3) {
        lmr = true;
        extension = -1;
      } 
      BoundedEval val = -negamax<CTO>(pos, id_d + extension, -beta, -t_alpha, do_quiesce);
      if (val.eval >= t_alpha && val.bound != BOUND_UPPER && lmr) {
        // if not a bad looking move re-search at unreduced depth for late move reductions
        extension++; 
        val = -negamax<CTO>(pos, id_d + extension, -beta, -t_alpha, do_quiesce);
      } 
      pos.unmakeMove();
      --current_depth;
      if (val.eval > best_so_far.eval && bsf_depth <= id_d + extension) {
        best_so_far = val;
        bsf_depth = id_d + extension;
      }
      // add 1 to "distance" if result is forced mate
      if (val.eval <= 40-INT32_MAX) val.eval++;
      if (val.eval >= INT32_MAX-40) val.eval--;
        
      b_evals[i++]=val;
      if (difftime(time(nullptr), init_time) >= time_limit) {
        BoundedEval r_eval = bestEvalInVector(b_evals);
        r_eval.bound = BOUND_LOWER; return r_eval;
      }
      if (val.eval > t_alpha) t_alpha = val.eval;
      // no fail if id_d < depth - 1, as we want to at least seach all nodes here for move ordering.
      if (t_alpha >= beta && id_d == depth - 1) {
        // at max depth of ids we may cut-off and return a lower bound
        return best_so_far;
      }
    }
    sortMoves(moves, b_evals);
  }

  return best_so_far;
}

}

#endif
