#include "transposition.h"

namespace Wyvern {
TranspositionTable::TranspositionTable(int b) {
  bits = b;
  size = 1ULL << bits;
  table = new Entry[size];
}
TranspositionTable::~TranspositionTable(){
  delete[] table;
}

BoundedEval TranspositionTable::lookup(U64 key, int depth) {
  Entry* tgt = table + (key & (size - 1));
  if (tgt->zobrist_key != key || depth > tgt->depth) return BoundedEval(BOUND_INVALID, 0);
  return tgt->value;
}

BoundedEval strongerBound(BoundedEval v1, BoundedEval v2) {
  if (v1.bound == BOUND_EXACT) return v1;
  if (v2.bound == BOUND_EXACT) return v2;
  if (v1.bound != v2.bound) return v1;
  if (v1.bound == BOUND_UPPER)
    return (v1.eval <= v2.eval) ? v1 : v2;
  else return (v1.eval >= v2.eval) ? v1 : v2;
}

void TranspositionTable::insert(U64 key, BoundedEval value, int depth) {
  Entry* tgt = table + (key & (size - 1));
  if (tgt->zobrist_key == key) {
    if (tgt->depth > depth) return;
    if (tgt->depth == depth) {
      tgt->value = strongerBound(tgt->value, value);
    }
  } 
  (*tgt) = Entry(key, value, depth);
}

}