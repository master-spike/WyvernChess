#include "position.h"
#include <iostream>

namespace Wyvern {


void Position::zobristHash() {
    U64 value = 0;
    U64 blacks = piece_colors[1];
    U64 whites = piece_colors[0];
    for (int i = 0; i < 6; i++) {
        U64 white_ps = pieces[i] & whites;
        U64 black_ps = pieces[i] & blacks;
        for (;white_ps; white_ps &= white_ps-1) {
            value ^= zobristNum(i, 0, __builtin_ctzll(white_ps));
        }
        for (;black_ps; black_ps &= black_ps-1) {
            value ^= zobristNum(i, 0, __builtin_ctzll(black_ps));
        }
    }
    value ^= zobristEP(ep_square);
    value ^= zobristCR(castling);
    if (tomove == COLOR_BLACK) value ^= zobristToMove();
    zobrist = value;
}

int Position::makeMove(U32 move) {
  int isq = move & 0x3F;
  int tsq = (move >> 6) & 0x3F;
  int promo_piece = ((move >> 12) & 3) + 1;
  int capture_piece = ((move>>17) & 7) - 1;
  U32 is_capture = YES_CAPTURE & move;
  int special = move & 0xC000;
  U64 ibb = 1ULL << isq;
  U64 tbb = 1ULL << tsq;
  int pt = ((move >> 20) & 7) - 1;
  cr_history.emplace_back(castling);
  hmc_history.emplace_back(fifty_half_moves);
  position_history.emplace_back(zobrist);
  // zobrist out old piece position
  zobrist ^= zobristNum(pt, tomove, isq);
  // hash-out old zobrist ep before setting to ep_square = 0
  zobrist ^= zobristEP(ep_square);
  ep_square = 0;
  fifty_half_moves++;
  if (pt == PAWN - 1) fifty_half_moves = 0;
  if (special == ENPASSANT) {
    U64 ep_tgt = (tomove) ? tbb << 8 : tbb >> 8;
    // zobrist in our target, and out enemy pawn
    zobrist ^= zobristNum(PAWN-1, tomove^1, __builtin_ctzll(ep_tgt));
    zobrist ^= zobristNum(PAWN-1, tomove, tsq);
    pieces[PAWN-1] ^= ibb ^ tbb ^ ep_tgt;
    piece_colors[tomove] ^= ibb ^ tbb;
    piece_colors[tomove^1] ^= ep_tgt;
  }
  if (special == NORMAL) {
    piece_colors[tomove] ^= ibb ^ tbb;
    zobrist ^= zobristNum(pt, tomove, tsq); // zobrist in our target
    pieces[pt] ^= ibb ^ tbb;
    if (pt == (PAWN-1) && ((ibb >> 16) == tbb || (tbb >> 16) == ibb))
      ep_square = 1ULL << ((isq + tsq)/2);
    if (pt == ROOK-1) {
      if (ibb & FILE_A & RANK_1) castling = (enum CastlingRights) (castling & ~CR_WQ);
      if (ibb & FILE_H & RANK_1) castling = (enum CastlingRights) (castling & ~CR_WK);
      if (ibb & FILE_A & RANK_8) castling = (enum CastlingRights) (castling & ~CR_BQ);
      if (ibb & FILE_H & RANK_8) castling = (enum CastlingRights) (castling & ~CR_BK);
    }
    if (pt == KING-1) {
      castling = (tomove) ? (enum CastlingRights) (castling & ~CR_BLACK)
                          : (enum CastlingRights) (castling & ~CR_WHITE);
    }
  }
  if (special == PROMO) {
    piece_colors[tomove] ^= ibb ^ tbb;
    pieces[promo_piece] ^= tbb;
    pieces[PAWN-1] ^= ibb;
    zobrist ^= zobristNum(promo_piece, tomove, tsq);
  }
  if (special == CASTLES) {
    zobrist ^= zobristNum(KING-1, tomove, tsq); // zobrist king target
    U64 castle_rank = (tomove == COLOR_WHITE) ? RANK_1 : RANK_8;
    U64 rook_isq = (ibb > tbb) ? castle_rank & FILE_A : castle_rank & FILE_H;
    U64 rook_tsq = (ibb > tbb) ? castle_rank & FILE_D : castle_rank & FILE_F;
    // zobrist rook move
    zobrist ^= zobristNum(ROOK-1,tomove, rook_isq) ^ zobristNum(ROOK-1,tomove, rook_tsq);
    pieces[ROOK-1] ^= rook_isq ^ rook_tsq;
    pieces[KING-1] ^= ibb ^ tbb;
    piece_colors[tomove] ^= rook_isq ^ rook_tsq ^ ibb ^ tbb;
    castling = (tomove == COLOR_WHITE) ? (enum CastlingRights) (castling & ~CR_WHITE)
                                       :(enum CastlingRights) (castling & ~CR_BLACK);
  }
  if (is_capture) {
    fifty_half_moves = 0;
    // zobrist out enemy piece
    zobrist ^= zobristNum(capture_piece, tomove^1, tsq);
    piece_colors[tomove^1] ^= tbb;
    pieces[capture_piece] ^= tbb;
      if (tbb & FILE_A & RANK_1) castling = (enum CastlingRights) (castling & ~CR_WQ);
      if (tbb & FILE_H & RANK_1) castling = (enum CastlingRights) (castling & ~CR_WK);
      if (tbb & FILE_A & RANK_8) castling = (enum CastlingRights) (castling & ~CR_BQ);
      if (tbb & FILE_H & RANK_8) castling = (enum CastlingRights) (castling & ~CR_BK);
  }
  // update zobrist cr and new ep square
  zobrist ^= zobristEP(ep_square);
  zobrist ^= zobristCR((enum CastlingRights) (castling ^ cr_history.back()));
  // new zobrist
  zobrist ^= zobristToMove();
  full_moves += tomove;
  tomove = (enum Color) (tomove ^ 1);
  move_history.emplace_back(move);
  return 0;
}

int Position::unmakeMove() {
  U32 move = move_history.back();
  enum PieceType captured_piece = (enum PieceType) ((move >> 17) & 7);
  zobrist = position_history.back();
  tomove = (enum Color) (tomove ^ 1);
  full_moves -= tomove;
  int isq = move & 0x3F;
  int tsq = (move >> 6) & 0x3F;
  U64 ibb = 1ULL << isq;
  U64 tbb = 1ULL << tsq;
  int promo_piece = ((move >> 12) & 3) + 1; 
  int special = move & 0xC000;
  int pt = ((move >> 20) & 7) - 1;
  ep_square = 0;
  if (YES_CAPTURE & move) {
    piece_colors[tomove^1] ^= tbb;
    pieces[captured_piece-1] ^= tbb;
  }
  if (special == NORMAL) {
    piece_colors[tomove] ^= ibb ^ tbb;
    pieces[pt] ^= ibb ^ tbb;
  }
  if (special == PROMO) {
    piece_colors[tomove] ^= ibb ^ tbb;
    pieces[promo_piece] ^= tbb;
    pieces[PAWN-1] ^= ibb;
  }
  if (special == CASTLES) {
    U64 castle_rank = (tomove == COLOR_WHITE) ? RANK_1 : RANK_8;
    U64 rook_isq = (ibb > tbb) ? castle_rank & FILE_A : castle_rank & FILE_H;
    U64 rook_tsq = (ibb > tbb) ? castle_rank & FILE_D : castle_rank & FILE_F;
    pieces[ROOK-1] ^= rook_isq ^ rook_tsq;
    pieces[KING-1] ^= ibb ^ tbb;
    piece_colors[tomove] ^= rook_isq ^ rook_tsq ^ ibb ^ tbb;
  }
  if (special == ENPASSANT) {
    ep_square = tbb;
    U64 ep_tgt = (tomove) ? tbb << 8 : tbb >> 8; 
    pieces[PAWN-1] ^= ibb ^ tbb ^ ep_tgt;
    piece_colors[tomove] ^= ibb ^ tbb;
    piece_colors[tomove^1] ^= ep_tgt;
  }
  else if (move_history.size() > 0) {
    // RECOVER PREVIOUS EN PASSANT SQUARE
    U32 prev_move = move_history.back();
    int pm_t = 63 & (prev_move >> 6);
    int pm_i = prev_move & 63;
    if ((prev_move & MOVE_ALL_PIECES) == MOVE_PAWN) {
      ep_square = (1ULL << ((pm_t + pm_i)/2)) & files[pm_i & 7];
    }
  }
  fifty_half_moves = hmc_history.back();
  hmc_history.pop_back();
  castling = cr_history.back();
  cr_history.pop_back();
  move_history.pop_back();
  position_history.pop_back();
  return 0;
}



enum Color Position::getToMove() const {
  return tomove;
}

std::vector<U64>::const_reverse_iterator Position::positionHistoryIteratorBegin()
{
  return position_history.crbegin();
}

std::vector<U64>::const_reverse_iterator Position::positionHistoryIteratorEnd()
{
  return position_history.crend();
}

U64 Position::getZobrist() const {
  return zobrist;
}

Position::Position()
    : piece_colors{0xFFFFULL, 0xFFFF000000000000ULL},
      ep_square(0),
      pieces{0x00FF00000000FF00ULL, 0x4200000000000042ULL,
             0x2400000000000024ULL, 0x8100000000000081ULL,
             0x0800000000000008ULL, 0x1000000000000010ULL},
      castling(CR_ANY),
      tomove(COLOR_WHITE),
      fifty_half_moves(0),
      full_moves(0),
      zobrist(0)
{
  U64 invalid = piece_colors[0] & piece_colors[1];
  invalid |= piece_colors[0] ^ piece_colors[1]
           ^ pieces[0] ^ pieces[1] ^ pieces[2]
           ^ pieces[3] ^ pieces[4] ^ pieces[5];
  if (invalid) std::cout << "DEFAULT POSITION CONSTRUCTOR BROKEN" << std::endl;
  zobristHash();
}

void Position::printFen() {
  
}

void Position::printPretty() {
  const char piece_chars[7] = {'?','P','N','B','R','Q','K'};
  const char dest_wsl = '(';
  const char dest_wsr = ')';
  const char dep_char = '+';
  U32 last_move = (move_history.empty()) ? MOVE_NONE : move_history.back();
  int isq = (last_move) ? last_move&63 : 64;
  int tsq = (last_move) ? (last_move >> 6) & 63 : 64;
  std::cout << "+---+---+---+---+---+---+---+---+ " << std::endl;
  for (int i = 7; i >= 0; --i) {
    std::cout << '|';
    for (int j = 0; j < 8; j++) {
      int p = i*8+j;
      char ws = ' ';
      if ((i+j)&1) ws = '/';
      char wsl = ws;
      char wsr = ws;
      if (i*8 + j == tsq) {
        wsl = dest_wsl;
        wsr = dest_wsr;
      }
      char piece = ws;
      if (i*8 + j == isq) piece = dep_char;
      if (piece_colors[COLOR_WHITE] & (1ULL << p))
        piece = piece_chars[pieceAtSquare(1ULL << p)];
      if (piece_colors[COLOR_BLACK] & (1ULL << p))
        piece = piece_chars[pieceAtSquare(1ULL << p)] + ('a' - 'A');
      std::cout << wsl << piece << wsr <<'|';
    }
    std::cout << "  ";
    if (i == 7) {
      std::cout << ((tomove) ? "Black" : "White") << " to move";
    }
    if (i <= 1) {
      for (int pt = 4; pt >= 0; pt--) {
        for (int j = 0; j < __builtin_popcountll(pieces[pt] & piece_colors[i]); ++j) {
          std::cout << (char) (piece_chars[pt+1] + ('a' - 'A')*i);
        }
      }
    }
    std::cout << std::endl << "+---+---+---+---+---+---+---+---+ " << std::endl;
  }

}
int Position::getHMC() const {
  return fifty_half_moves;
}
int Position::getFMC() const {
  return full_moves;
}
enum CastlingRights Position::getCR() const {
  return castling;
}
U64* Position::getPieceColors() {
  return piece_colors;
}
U64* Position::getPieces() {
  return pieces;
}
enum PieceType Position::pieceAtSquare(U64 sq) {
  for (int i = 0; i < 6; i++) {
    if (sq & pieces[i]) return (enum PieceType) (i+1);
  }
  return PIECE_NONE;
}

U64 Position::getEpSquare() {
  return ep_square;
}

int Position::checkValidity() {
  if (piece_colors[0] & piece_colors[1]) {
    std::cout << "Position bad. White pieces: \n";
    printbb(piece_colors[COLOR_WHITE]);
    std::cout << "Black pieces: \n";
    printbb(piece_colors[COLOR_BLACK]);
    return 1;
  }
  U64 all_pieces = 0;
  for (int i = 0; i < KING; i++) {
    if (pieces[i] & all_pieces) {
      std::cout << "Position bad: "<< i <<"\n";
      printbb(pieces[i]);
      std::cout << "overlap:\n";
      printbb(all_pieces);
      return 1;
    }
    all_pieces |= pieces[i];
  }
  U64 wkingpos = pieces[KING-1] & piece_colors[COLOR_WHITE];
  U64 bkingpos = pieces[KING-1] & piece_colors[COLOR_BLACK];
  if (__builtin_popcountll(wkingpos) != 1) {
    printbb(wkingpos);
    return 1;
  }
  if (__builtin_popcountll(bkingpos) != 1) {
    printbb(bkingpos);
    return 1;
  }
  return 0;
}

Position::Position(const Position& pos) {
  for (int i = 0; i < 2; i++) {
    piece_colors[i] = pos.piece_colors[i];
  }
  for (int i = 0; i < 6; i++) {
    pieces[i] = pos.pieces[i];
  }
  ep_square = pos.ep_square;
  castling = pos.castling;
  tomove = pos.tomove;
  fifty_half_moves = pos.fifty_half_moves;
  full_moves = pos.full_moves;
  zobrist = pos.zobrist;
  move_history = std::vector<U32>(pos.move_history);
  position_history = std::vector<U64>(pos.position_history);
  cr_history = std::vector<enum CastlingRights>(pos.cr_history);
}

static int fenParseBoardChar(char c) {
  if ('1' <= c && c <= '8') {
    return (c - '0');
  }
  if (c == '/') {
    return 16;
  }
  switch (c)
  {
  case 'p':
      return 32 + PAWN*64;
  case 'n':
      return 32 + KNIGHT*64;
  case 'b':
      return 32 + BISHOP*64;
  case 'r':
      return 32 + ROOK*64;
  case 'q':
      return 32 + QUEEN*64;
  case 'k':
      return 32 + KING*64;
  case 'P':
      return PAWN*64;
  case 'N':
      return KNIGHT*64;
  case 'B':
      return BISHOP*64;
  case 'R':
      return ROOK*64;
  case 'Q':
      return QUEEN*64;
  case 'K':
      return KING*64;
  }
  return 0;
}

Position::Position(char* fen) {
  int rank = 7;
  int file = 0;
//  int num = 0;
  castling = CR_ANY;
  tomove = COLOR_WHITE;
  fifty_half_moves = 0;
  full_moves = 0;
  ep_square = 0;
  for (int i = 0; i<6; i++) {
    pieces[i] = 0;
  }
  piece_colors[0] = 0;
  piece_colors[1] = 0;
  while((*fen) != '\0') {
    if (file > 8) return;
    int fc = fenParseBoardChar(*fen);
    if (!fc) return;
    if (fc == 16) {
      if (file != 8) return;
      file = 0;
      --rank;
    }
    else if (file >= 8) return;
    else if (fc & 15) {
      file += (fc & 15);
    }
    else {
      int color = (fc/32)&1;
      int piece = (fc/64)-1;
      pieces[piece] |= 1ULL << (file + 8*rank);
      piece_colors[color] |= 1ULL << (file + 8*rank);
      file++;
    }
    fen++;

  }
  zobristHash();
}

}
