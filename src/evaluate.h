#ifndef H_GUARD_EVALUATE
#define H_GUARD_EVALUATE

#include "position.h"
#include "utils.h"
#include "types.h"

namespace Wyvern {

constexpr int pval_pawn = 100;
constexpr int pval_knight = 300;
constexpr int pval_bishop = 300;
constexpr int pval_rook = 500;
constexpr int pval_queen = 900;

constexpr int pvals[5] = {pval_pawn,pval_knight,pval_bishop,pval_rook,pval_queen};


// evaluates position for player to move
int evalMaterialOnly(Position& pos) {
  int player = pos.getToMove();
  int opponent = pos.getToMove() ^ 1;
  U64* pcols = pos.getPieceColors();
  U64* pcs = pos.getPieces();
  int total = 0;
  for (int i = 0; i < 5; ++i) {
    total += (popCount64(*(pcols+player) & pcs[i]) - popCount64(*(pcols+opponent) & pcs[i]))* pvals[i];
  }
  return total;
}

}



#endif


