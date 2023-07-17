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


int Search::perft(Position& pos, int depth, int max_depth) {
  Position ref_pos(pos);
  enum Color ct = pos.getToMove();
  if (depth == 0) return 1;
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
    */

    
    pos.makeMove(move);
    sum += perft(pos, depth-1, max_depth);
    pos.unmakeMove();
    if (!(ref_pos == pos)){
      std::cout << "make unmake move fucked up:" << std::endl;
      printSq(move & 63);
      std::cout << ':';
      printSq((move >> 6) & 63);
      std::cout << " " << std::hex << move << '\n' << std::dec;
      pos.printPretty();
      break;
    }
  }
  if (sum > 0) return sum;
  return 1;
}

}
