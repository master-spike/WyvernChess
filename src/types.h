#ifndef H_GUARD_TYPES
#define H_GUARD_TYPES

#include <cstdint>
#include <cstdlib>


typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t  U8;

namespace Wyvern {

  enum Color {COLOR_WHITE = 0, COLOR_BLACK = 1, COL_UNDEF = 2};
  enum Move : int {MOVE_NONE = 0, MOVE_NULL = 65};
  enum MoveType {NORMAL = 0, PROMO = 1 << 14, ENPASSANT = 2 << 14, CASTLES = 3 << 14};
  
  enum PieceType {PIECE_NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, INVALID = 8};
  
  enum CastlingRights {
    CR_NONE = 0, CR_WK = 1, CR_WQ = 2, CR_BK = 4, CR_BQ = 8,
    
    CR_KING = CR_WK | CR_BK,
    CR_QUEEN = CR_WQ | CR_BQ,
    CR_WHITE = CR_WK | CR_WQ,
    CR_BLACK = CR_BK | CR_BQ,
    CR_ANY = CR_WHITE | CR_BLACK,
    
    CR_UNDEF = 16 
  };

  enum MoveFilters : U32 {
    ALL_MOVES = UINT32_MAX,
    FMOVE_QUIETS = 1
  };

  enum File : U64 {
    FILE_A = 0x0101010101010101ULL,
    FILE_B = FILE_A << 1,
    FILE_C = FILE_A << 2,
    FILE_D = FILE_A << 3,
    FILE_E = FILE_A << 4,
    FILE_F = FILE_A << 5,
    FILE_G = FILE_A << 6,
    FILE_H = FILE_A << 7
  };
  enum Rank : U64 {
    RANK_1 = 0xFFULL,
    RANK_2 = RANK_1 << 8,
    RANK_3 = RANK_1 << 16,
    RANK_4 = RANK_1 << 24,
    RANK_5 = RANK_1 << 32,
    RANK_6 = RANK_1 << 40,
    RANK_7 = RANK_1 << 48,
    RANK_8 = RANK_1 << 56
  };

  const U64 files[8] = {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};
  const U64 ranks[8] = {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};
  
}


#endif
