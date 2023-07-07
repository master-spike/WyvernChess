#include "search.h"

namespace Wyvern {

int Search::negamax(Position& pos, int depth, int alpha, int beta) {
  // if terminal return draw, win, loss as appropriate
  // todo if
  U16 moves[220];
  int num_moves = movegen.generateMoves(pos, moves, true);
  if (num_moves == 0) {
    //terminal, return mate or stalemate
  }
  if (pos.getHMC() >= 50) return 0;
  if (depth == 0) {
    // return static eval
  } 
  int best_val = alpha;
  for (U16 move : moves) {
    pos.makeMove(move);
    int val = negamax(pos, depth - 1, -beta, -alpha);
    alpha = (val > alpha) ? val : alpha;
    if (alpha >= beta) break;
    best_val = (val > best_val) ? val : best_val;
    pos.unmakeMove();
  }
  return best_val;
}


}