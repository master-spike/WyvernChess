#ifndef H_GUARD_SEARCH
#define H_GUARD_SEARCH

#include "position.h"
#include "types.h"
#include "movegen.h"

namespace Wyvern {

class Search {
private:
  MoveGenerator movegen;
public:
  Search() = default;
  template<enum Color CT>
  int negamax(Position& pos, int depth, int alpha, int beta);
  ~Search() = default;
  int perft(Position &pos, int depth, int max_depth, int* n_capts, int* n_enpass, int* n_promo, int* n_castles);
};
}


#endif
