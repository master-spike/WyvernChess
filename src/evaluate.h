#ifndef H_GUARD_EVALUATE
#define H_GUARD_EVALUATE

#include "position.h"
#include "utils.h"
#include "types.h"

namespace Wyvern {

constexpr int pval_pawn = 100;
constexpr int pval_knight = 315;
constexpr int pval_bishop = 330;
constexpr int pval_rook = 500;
constexpr int pval_queen = 900;

constexpr int pvals[5] = {pval_pawn,pval_knight,pval_bishop,pval_rook,pval_queen};


// evaluates position for player to move

class Evaluator {
  public:
    int evalMaterialOnly(Position& pos);
    int evalPositional(Position &pos);
};

}



#endif


