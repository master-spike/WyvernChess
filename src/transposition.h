#pragma once

#include <cstddef>
#include <vector>

#include "types.h"

namespace Wyvern
{

class TranspositionTable
{
private:
  static constexpr int default_bits = 23; // ~200MB

  struct Entry
  {
    U64 zobrist_key = 0;
    BoundedEval value = BoundedEval(BOUND_INVALID, 0);
    int depth = -1;

    Entry() = default;
    Entry(U64 zk, BoundedEval v, int d) : zobrist_key(zk), value(v), depth(d) {}
  };

  int bits;
  std::vector<Entry> table;

public:
  BoundedEval lookup(U64 key, int depth);
  void insert(U64 key, BoundedEval value, int depth);

  TranspositionTable() : TranspositionTable(default_bits) {}
  explicit TranspositionTable(int b);
  ~TranspositionTable() = default;
  TranspositionTable(TranspositionTable&& tt) noexcept = default;
  TranspositionTable& operator=(TranspositionTable&&) noexcept = default;
  TranspositionTable& operator=(TranspositionTable const& tt) = delete;
  TranspositionTable(TranspositionTable const& tt) = delete;
};

} // namespace Wyvern
