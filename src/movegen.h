#ifndef H_GUARD_MOVEGEN
#define H_GUARD_MOVEGEN

#include "position.h"
#include "types.h"
#include "magicbb.h"

namespace Wyvern {


/*

move generator is responsible for putting valid moves in a Position into a move array U16* in preference order.
search can then pop these moves.
*/
 
class MoveGenerator {
private:
  template<enum PieceType PT> U64 bbPseudoLegalMoves(int p, const U64& postmask, const U64& bb_blockers);
  template<enum Color CT> U64 bbCastles(const Position& pos, const U64& checkmask);
  template<enum Color CT> U64 bbPawnLeftCaptures(const U64& pawns, const U64& postmask, const U64& capturables);
  template<enum Color CT> U64 bbPawnRightCaptures(const U64& pawns, const U64& postmask, const U64& capturables);
  template<enum Color CT> U64 bbPawnSinglePushes(const U64& pawns, const U64& postmask);
  template<enum Color CT> U64 bbPawnDoublePushes(const U64& pawns, const U64& postmask, const U64& bb_blockers);
  template<enum Color CT> U64 bbEnpassant(const Position& pos); // bits here are for the pawns that can DO enpassant
  int putMoves(int p, U64& targets, U16* outMoves);
  U64 knight_attack_table[64];
  U64 king_attack_table[64];
  MagicBB rook_magics[64];
  MagicBB bishop_magics[64];
public:
  MoveGenerator();
  int generateMoves(Position& pos, U16* outMoves, bool quiets = true);
  ~MoveGenerator();
};

}
#endif
