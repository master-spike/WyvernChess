#include "transposition.h"

namespace Wyvern
{

TranspositionTable::TranspositionTable(int b) : bits(b), table(1ULL << bits) {}

BoundedEval TranspositionTable::lookup(U64 key, int depth)
{
  const auto mask = static_cast<U64>(table.size() - 1);
  const Entry& tgt_deep = table[(key & mask) & ~1ULL];
  const Entry& tgt_shallow = table[(key & mask) | 1ULL];
  if (tgt_shallow.zobrist_key == key && depth <= tgt_shallow.depth)
    return tgt_shallow.value;
  if (tgt_deep.zobrist_key == key && depth <= tgt_deep.depth)
    return tgt_deep.value;
  return BoundedEval(BOUND_INVALID, 0);
}

BoundedEval strongerBound(BoundedEval v1, BoundedEval v2)
{
  if (v1.bound == BOUND_EXACT)
    return v1;
  if (v2.bound == BOUND_EXACT)
    return v2;
  if (v1.bound != v2.bound)
    return v1;
  if (v1.bound == BOUND_UPPER)
    return (v1.eval <= v2.eval) ? v1 : v2;
  else
    return (v1.eval >= v2.eval) ? v1 : v2;
}

void TranspositionTable::insert(U64 key, BoundedEval value, int depth)
{
  const auto mask = static_cast<U64>(table.size() - 1);
  Entry& tgt_deep = table[(key & mask) & ~1ULL];
  Entry& tgt_shallow = table[(key & mask) | 1ULL];
  if (tgt_deep.zobrist_key == key)
  {
    if (tgt_deep.depth < depth)
      tgt_deep.value = value;
    if (tgt_deep.depth == depth)
    {
      tgt_deep.value = strongerBound(tgt_deep.value, value);
    }
  }
  tgt_shallow = Entry(key, value, depth);
}

} // namespace Wyvern
