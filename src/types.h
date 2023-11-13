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
  enum PieceType {PIECE_NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, INVALID = 8};
  enum Move : U32 {MOVE_NONE = 0, MOVE_NULL = 65};
  enum MoveType : U32 {NORMAL = 0, PROMO = 1 << 14, ENPASSANT = 2 << 14, CASTLES = 3 << 14, MOVE_SPECIAL = 3 << 14};
  enum MoveCapture : U32 {
    NO_CAPTURE = 0,
    YES_CAPTURE = 1<<16,
    CAPTURE_PAWN = (PAWN << 17) | YES_CAPTURE,
    CAPTURE_KNIGHT = (KNIGHT << 17) | YES_CAPTURE,
    CAPTURE_BISHOP = (BISHOP << 17) | YES_CAPTURE,
    CAPTURE_ROOK = (ROOK << 17) | YES_CAPTURE,
    CAPTURE_QUEEN = (QUEEN << 17) | YES_CAPTURE,
    CAPTURE_KING = (KING << 17) | YES_CAPTURE
  };
  enum MovePiece  : U32 {
    MOVE_PAWN = PAWN << 20,
    MOVE_KNIGHT = KNIGHT << 20,
    MOVE_BISHOP = BISHOP << 20,
    MOVE_ROOK = ROOK << 20,
    MOVE_QUEEN = QUEEN << 20,
    MOVE_KING = KING << 20,
    MOVE_ALL_PIECES = 7 << 20
  };
  enum CastlingRights : U16 {
    CR_NONE = 0, CR_WK = 1, CR_WQ = 2, CR_BK = 4, CR_BQ = 8,
    
    CR_KING = CR_WK | CR_BK,
    CR_QUEEN = CR_WQ | CR_BQ,
    CR_WHITE = CR_WK | CR_WQ,
    CR_BLACK = CR_BK | CR_BQ,
    CR_ANY = CR_WHITE | CR_BLACK,
    
    CR_UNDEF = 16 
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

  enum Bound : int {
    BOUND_EXACT = 0,
    BOUND_UPPER = 1,
    BOUND_LOWER = -1,
    BOUND_INVALID = 2
  };

  struct BoundedEval {
    enum Bound bound;
    int eval;
    BoundedEval operator -() {
      return BoundedEval((enum Bound)(-bound), -eval);
    }
    bool operator ==(BoundedEval& bv) {
      if (bound != bv.bound) return false;
      if (eval != bv.eval) return false;
      return true;
    }
    BoundedEval(enum Bound b, int e) {
      bound = b; eval = e;
    }
    BoundedEval() = default;
  };

  const U64 files[8] = {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};
  const U64 ranks[8] = {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};
  
}


#endif
