#ifndef H_GUARD_MAGIC
#define H_GUARD_MAGIC

#include "types.h"
#include "utils.h"

#define MAGIC_MAX_TRIALS 0x10000000000ULL

namespace Wyvern 
{
static const int magicRBits[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12};

static const int magicBBits[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

class MagicBB {
  private:
    int square;
    U64 mask;
    U64 magicnum;
    U64* table;
    int bits;
  public:
    MagicBB() : square(0), magicnum(0), mask(0), table(nullptr), bits(0){};
    MagicBB(int sq, U64 mg, U64 ms, U64* tb, int b);
    U64 compute(U64 blockers);
    
    template<enum PieceType PT>
    int initialise(); // returns 1 on failure
  };

  template<enum PieceType PT>
  U64 findMagicNum(int p);

  template<enum PieceType PT>
  U64 generateAttacks(int p, U64 blockers);
  
  U64* initialiseAllMagics(MagicBB* bishops, MagicBB* rooks);

}


#endif