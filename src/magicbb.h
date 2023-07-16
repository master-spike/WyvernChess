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

template<enum PieceType PT>
U64 getPremask(int p);

class MagicBB {
  private:
    int square;
    U64 mask;
    U64 magicnum;
    U64* table;
    int bits;
  public:
    MagicBB() = default;
    MagicBB(int sq, U64 mg, U64 ms, U64* tb, int b);
    U64 compute(U64 blockers);
    
    template<enum PieceType PT>
    int initialise(); // returns 1 on failure
  };

template<enum PieceType PT>
U64 findMagicNum(int p) {
  if constexpr(PT != ROOK && PT != BISHOP) return 0;
  int bits = (PT == BISHOP) ? magicBBits[p] : magicRBits[p];
  U64 mask = getPremask<PT>(p);
  U64* temp_table = (U64*) malloc(sizeof(U64) *(size_t) (1ULL << bits));
  for (U64 k = 0; k < MAGIC_MAX_TRIALS; ++k) {
    U64 magic = rand64() & rand64() & rand64() ;
    if (popCount64((magic*mask) & 0xFF00000000000000ULL) < 6) continue;
    MagicBB mbb(p , magic, mask, temp_table, bits);
    if (!mbb.initialise<PT>()) {
      free(temp_table);
      //std::cout << "magic number " << p << " for " << ((PT == ROOK) ? "  rook" : "bishop") << " found in " << k+1 << " tries" << "\n";
      return magic;
    }
  }
  std::cout << "Failed to find magic number " << p << " for " << ((PT == ROOK) ? "  rook" : "bishop") << "\n";
  free(temp_table);
  return 0;
}

  template<enum PieceType PT>
  U64 generateAttacks(int p, U64 blockers);
  
  // returns a pointer to the allocated memory segment for all the lookup tables for deletion
  // by anything which uses it. This is probably not good practice.
  U64* initialiseAllMagics(MagicBB* bishops, MagicBB* rooks);

}


#endif
