#ifndef H_GUARD_TRANSPOSITION
#define H_GUARD_TRANSPOSITION

#include "types.h"
#define TRANSPOSITION_TABLE_DEFAULT_BITS 16; // ~1.5MB

namespace Wyvern {

class TranspositionTable {
private:
  struct Entry {
    U64 zobrist_key;
    BoundedEval value;
    int depth;
    Entry() = default;
    Entry(U64 zk, BoundedEval v, int d) {
      zobrist_key = zk;
      value = v;
      depth = d;
    }
    ~Entry() = default;
    Entry(Entry& e) = delete;
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
  TranspositionTable(TranspositionTable& tt) = delete;

};

}

#endif