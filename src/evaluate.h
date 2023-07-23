#ifndef H_GUARD_EVALUATE
#define H_GUARD_EVALUATE

#include "position.h"
#include "utils.h"
#include "types.h"
#include "magicbb.h"
#include <memory>

namespace Wyvern {

constexpr int pval_pawn = 100;
constexpr int pval_knight = 315;
constexpr int pval_bishop = 330;
constexpr int pval_rook = 500;
constexpr int pval_queen = 900;

constexpr int pvals[6] = {pval_pawn,pval_knight,pval_bishop,pval_rook,pval_queen, 20000}; // king has absurd value for see

constexpr int endgame_material_limit = 20;
constexpr int midgame_material_limit = 46;

// evaluates position for player to move

class Evaluator {
private:
  // 0-63 for white, 64-127 for black
  // U64 bb_passed_pawns[128];
  std::shared_ptr<MagicTable> mt;

public:
  Evaluator() = delete;
  int evalMaterialOnly(Position& pos);
  int totalMaterial(Position &pos);
  int evalPositional(Position &pos);
  Evaluator(std::shared_ptr<MagicTable> mt);
  ~Evaluator() = default;
  Evaluator(Evaluator& evaluator) = delete;
  
  template<enum Color CT>
  int seeCapture(Position& pos, U32 capture);
  int see(Position& pos, enum PieceType piece, enum PieceType target, int frsq, int tosq, int side);
};

template<enum Color CT>
int Evaluator::seeCapture(Position& pos, U32 capture) {
  if (!(capture & YES_CAPTURE)) return 0;
  
  enum PieceType target = (enum PieceType) ((capture >> 17) & 7);
  enum PieceType aPiece = (enum PieceType) ((capture >> 20) & 7);
  int frsq = capture & 63;
  int tosq = (capture >> 6) & 63;

  return see(pos, aPiece, target, frsq, tosq, (int) CT);
}

}



#endif
