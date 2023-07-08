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
  
}


#endif
