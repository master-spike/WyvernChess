#ifndef H_GUARD_SEARCH
#define H_GUARD_SEARCH

#include "position.h"
#include "types.h"
#include "movegen.h"
#include "evaluate.h"
#include "transposition.h"
#include <ctime>

namespace Wyvern {

constexpr int qs_depth_hardlimit = 10;

class Search {
private:
  std::shared_ptr<MagicTable> mt;
  Evaluator evaluator;
  MoveGenerator movegen;
  void sortMoves(std::vector<U32>& moves, std::vector<BoundedEval>& evals);
  template<enum Color CT>
  BoundedEval quiesce(Position pos, int alpha, int beta, int depth_hard);
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
  U32 bestmove(Position pos, double t_limit, int max_basic_depth, int max_depth_hard, int& out_eval);
  template<enum Color CT>
  BoundedEval negamax(Position& pos, int depth, int alpha, int beta,  bool do_quiesce, int d_max);
  ~Search() = default;
  U64 perft(Position &pos, int depth, int* n_capts, int* n_enpass, int* n_promo, int* n_castles, int* checks);
};

template<enum Color CT> 
BoundedEval Search::quiesce(Position pos, int alpha, int beta, int depth_hard) {
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

  int stand_pat_initial = stand_pat;

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
  enum Bound bound = (alpha > stand_pat) ? BOUND_UPPER : BOUND_EXACT;
  
  if (depth_hard == 0) {
    return BoundedEval(BOUND_UPPER, stand_pat);
  }

  // lookup from table, updating alpha and beta and returning if outside bounds or exact
  // this logic is needed if we are using aspirational windows

  // if we are shallow do not waste time searching the transposition table,
  // as seaching it at every node is cache-unfriendly
  BoundedEval table_lookup = (depth_hard > qs_depth_hardlimit - 4)
                           ? ttable.lookup(pos.getZobrist(), 0)
                           : BoundedEval(BOUND_INVALID, 0);
  if (!(table_lookup.bound & BOUND_INVALID)) {
    ++table_hits; 
    if (table_lookup.bound == BOUND_EXACT) return table_lookup;
    if (table_lookup.bound == BOUND_UPPER) {
      if (table_lookup.eval <= alpha) return table_lookup;
      if (table_lookup.eval <= beta) beta = table_lookup.eval;
    }
    if (table_lookup.bound == BOUND_LOWER) {
      if (table_lookup.eval >= beta) return table_lookup;
      if (table_lookup.eval >= alpha) alpha = table_lookup.eval;
    }
  }

  // now we do captures.
  for (U32 move : moves) {
    
    if (!checks && (move & YES_CAPTURE) && !((move & MOVE_SPECIAL) == PROMO)) {
      // skip bad captures - delta pruning outside the endgame, or <0 any time
      pos.makeMove(move);
      U64 move_is_check = movegen.inCheck(pos);
      pos.unmakeMove();
      if (!move_is_check) {
        int seeval = evaluator.seeCapture<CT>(pos, move);
        if (seeval < 0) continue;
        if (evaluator.totalMaterial(pos) > endgame_material_limit) {
          int delta = seeval + stand_pat_initial + 200;
          if (delta < alpha) continue;
        }
      }
    }
    
    pos.makeMove(move);
    ++current_depth;
    BoundedEval val = -quiesce<CTO>(pos, -beta, -alpha, depth_hard-1);
    pos.unmakeMove();
    --current_depth;
    // prefer further mates / closer mates
    if (val.eval <= 40-INT32_MAX) val.eval++;
    if (val.eval >= INT32_MAX-40) val.eval--;

    alpha = (val.eval > alpha) ? val.eval : alpha;
    stand_pat = (val.eval > stand_pat) ? val.eval : stand_pat;
    if (val.eval >= alpha) bound = BOUND_EXACT;
    if (alpha >= beta) {
      //std::cout << "cutoff" << std::endl;
      bound = BOUND_LOWER; break;
    }
  }
  ttable.insert(pos.getZobrist(),BoundedEval(bound, stand_pat),0);

  return BoundedEval(bound, stand_pat);

}
template<enum Color CT>
BoundedEval Search::negamax(Position& pos, int depth, int alpha, int beta, bool do_quiesce, int d_max) {

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
  if (depth == 0 || d_max == 0) {
    if (!do_quiesce) return BoundedEval(BOUND_EXACT, evaluator.evalPositional(pos));
    return quiesce<CT>(pos, alpha, beta, qs_depth_hardlimit);
  }

  // lookup from table, updating alpha and beta and returning if outside bounds or exact
  // this logic is needed if we are using aspirational windows
  BoundedEval table_lookup = ttable.lookup(pos.getZobrist(), depth);
  if (!(table_lookup.bound & BOUND_INVALID)) {
    ++table_hits;
    if (table_lookup.bound == BOUND_EXACT) return table_lookup;
    if (table_lookup.bound == BOUND_UPPER) {
      if (table_lookup.eval <= alpha) return table_lookup;
      if (table_lookup.eval <= beta) beta = table_lookup.eval;
    }
    if (table_lookup.bound == BOUND_LOWER) {
      if (table_lookup.eval >= beta) return table_lookup;
      if (table_lookup.eval >= alpha) alpha = table_lookup.eval;
    }
  }

  std::vector<BoundedEval> b_evals(moves.size(), BoundedEval(BOUND_UPPER, -INT32_MAX));
  BoundedEval best_so_far = BoundedEval(BOUND_EXACT, -INT32_MAX);
  int bsf_depth = -1;

  // iterative deepening up to depth-2 to get promising move order 
  for (int id_d = 0; id_d < depth; id_d++) {
    int t_alpha = alpha; // temporary value of alpha for ids
    int i = 0;
    for (U32 move : moves) {

      current_depth++;
      int extension = 0;

      if (move & YES_CAPTURE) {
        // check see for good capture
        if (evaluator.seeCapture<CT>(pos, move) >= 0) extension = 1;
      }
      // extend search if move is check
      if (!(extension) && movegen.inCheck(pos)) extension = 1;
      pos.makeMove(move);
      // or if giving check
      if (!(extension) && movegen.inCheck(pos)) extension = 1;
      //passed pawn push
      //U64 enemy_pawns = pos.getPieceColors()[CT^1] & pos.getPieces()[PAWN-1];
      //if ((move >> 20 & 7) == PAWN && !(enemy_pawns & mt->passed_pawns[64*CT + (move&63)])) extension = 1;
      
      bool lmr = false;
      
      // do not do lmr on good captures, checks, check evasions, shallow depth searches, early moves.
      if (!extension && !((move & MOVE_SPECIAL) == PROMO) && i >= 4 && id_d > 2) {
        lmr = true;
        extension = -1;
      }
      
      BoundedEval val = -negamax<CTO>(pos, id_d + extension, -beta, -t_alpha, do_quiesce, d_max-1);
      if (val.eval >= t_alpha && val.bound != BOUND_UPPER && lmr) {
        // if not a bad looking move re-search at unreduced depth for late move reductions
        extension++; 
        val = -negamax<CTO>(pos, id_d + extension, -beta, -t_alpha, do_quiesce, d_max-1);
      } 
      pos.unmakeMove();
      --current_depth;
      // add 1 to "distance" if result is forced mate
      if (val.eval <= 40-INT32_MAX) val.eval++;
      if (val.eval >= INT32_MAX-40) val.eval--;

      if (val.eval > best_so_far.eval && bsf_depth <= id_d + extension) {
        best_so_far = val;
        bsf_depth = id_d + extension;
      }

        
      b_evals[i++]=val;
      if (difftime(time(nullptr), init_time) >= time_limit) {
        return best_so_far;
      }
      if (val.eval > t_alpha) t_alpha = val.eval;
      // no fail if id_d < depth - 1, as we want to at least seach all nodes here for move ordering.
      if (t_alpha >= beta) {
        best_so_far.bound = BOUND_LOWER;
        break; // either continue ids or
      }
    }
    sortMoves(moves, b_evals);
  }
  if (best_so_far.eval < alpha) best_so_far.bound = BOUND_UPPER;
  else best_so_far.bound = BOUND_EXACT;
  ttable.insert(pos.getZobrist(),best_so_far,depth);
  return best_so_far;
}

}

#endif
