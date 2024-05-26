#ifndef H_GUARD_TRANSPOSITION
#define H_GUARD_TRANSPOSITION

#include "types.h"
#define TRANSPOSITION_TABLE_DEFAULT_BITS 23; // ~200MB

namespace Wyvern {

class TranspositionTable {
private:
  struct Entry {
    U64 zobrist_key;
    BoundedEval value;
    int depth;
    Entry() {
      depth = -1;
      zobrist_key = 0;
    }
    Entry(U64 zk, BoundedEval v, int d) {
      zobrist_key = zk;
      value = v;
      depth = d;
    }
  };
  int bits;
  size_t size;
  Entry* table;
public:
  BoundedEval lookup(U64 key, int depth);
  void insert(U64 key, BoundedEval value, int depth);
  
  TranspositionTable() {
    bits = TRANSPOSITION_TABLE_DEFAULT_BITS;
    size = 1ULL << bits;
    table = new Entry[size];
  }

  TranspositionTable(int b);
  ~TranspositionTable();
  TranspositionTable(TranspositionTable&& tt) = delete;
  TranspositionTable operator=(TranspositionTable&&) = delete;
  TranspositionTable operator=(TranspositionTable const& tt) = delete;
  TranspositionTable(TranspositionTable const& tt) = delete;

};

}

#endif
