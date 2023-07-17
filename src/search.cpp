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


int Search::perft(Position& pos, int depth, int max_depth, int* n_capts, int* n_enpass, int* n_promo, int* n_castles, int* checks) {
  Position ref_pos(pos);
  U64* pcols = pos.getPieceColors();
  U64* pcs = pos.getPieces();
  enum Color ct = pos.getToMove();
  int our_kp = __builtin_ctzll(pcs[KING-1] & pcols[ct]);
  int opp_kp = __builtin_ctzll(pcs[KING-1] & pcols[ct^1]);
  if (__builtin_popcountll(pcs[KING-1] & pcols[ct]) != 1) return 1;
  if (__builtin_popcountll(pcs[KING-1] & pcols[ct^1]) != 1) return 1;
  U64 checkers = (ct) ? movegen.squareAttackedBy<COLOR_WHITE>(our_kp, pos, 0)
                      : movegen.squareAttackedBy<COLOR_BLACK>(our_kp, pos, 0);
  if (checkers) (*checks)++;
  if (checkers) {
    pos.printPretty();
    printbb(checkers);
  }
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
    if (move & YES_CAPTURE) ++(*n_capts);
    if ((move & MOVE_SPECIAL) == ENPASSANT) ++(*n_enpass);
    if ((move & MOVE_SPECIAL) == PROMO) ++(*n_promo);
    if ((move & MOVE_SPECIAL) == CASTLES) ++(*n_castles);

    pos.makeMove(move);
    enum Color kat = COL_UNDEF;
    int bad_kp = 0;
    if (__builtin_popcountll(pcs[KING-1] & pcols[ct^1]) != 1) {
      kat = ct; bad_kp = opp_kp;
    }
    else if (__builtin_popcountll(pcs[KING-1] & pcols[ct^1]) != 1) {
      kat = (enum Color)(ct^1); bad_kp = our_kp;
    }
    if (kat != COL_UNDEF) {
      for (U32 m : pos.move_history) {
        std::cout<<"(";
        printSq(m & 63);
        printSq((m >> 6) & 63);
        std::cout<<") ";
      }
      std::cout << " " << std::hex << move << '\n' << std::dec;
      pos.printPretty();
      ref_pos.printPretty();
      U64 kattack = (kat) ? movegen.squareAttackedBy<COLOR_WHITE>(bad_kp, ref_pos, 0):
                           movegen.squareAttackedBy<COLOR_BLACK>(bad_kp, ref_pos, 0);
      printbb(kattack);
      U64 input;
      std::cin >> std::hex >> input;
      std::cout << input << std::endl;
      pos.unmakeMove();
      return 0;
    }
    sum += perft(pos, depth-1, max_depth, n_capts, n_enpass, n_promo, n_castles, checks);
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
