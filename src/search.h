#ifndef H_GUARD_SEARCH
#define H_GUARD_SEARCH

#include "position.h"
#include "types.h"
#include "movegen.h"
#include "utils.h"

namespace Wyvern {

typedef struct MoveEval {
  int eval;
  U16 move;
};

class Search {
private:
  Position position;
  MoveGenerator movegen;
public:
  Search() = default;
  Search(const Position& pos);

  template<enum Color CT>
  int negamax(Position& pos, int depth, int alpha, int beta);
  ~Search() = default;
};
  
}


#endif
