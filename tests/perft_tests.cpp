#include "position.h"
#include "search.h"
#include "transposition.h"

#include <iostream>
#include <string>
#include <string_view>

namespace
{

constexpr char kiwipete_fen[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R";

struct PerftResult
{
  U64 nodes;
  int captures;
  int en_passant;
  int castles;
  int promotions;
  int checks;
};

PerftResult run_perft(Wyvern::Search& search, Wyvern::Position& position, int depth)
{
  PerftResult result{0, 0, 0, 0, 0, 0};
  result.nodes = search.perft(position, depth, &result.captures, &result.en_passant,
                              &result.promotions, &result.castles, &result.checks);
  return result;
}

bool expect_eq(std::string_view name, U64 actual, U64 expected)
{
  if (actual == expected)
  {
    return true;
  }

  std::cerr << name << ": expected " << expected << ", got " << actual << '\n';
  return false;
}

bool expect_bound(std::string_view name, Wyvern::Bound actual, Wyvern::Bound expected)
{
  if (actual == expected)
  {
    return true;
  }

  std::cerr << name << ": expected " << static_cast<int>(expected) << ", got "
            << static_cast<int>(actual) << '\n';
  return false;
}

bool expect_perft(std::string_view name, const PerftResult& actual, const PerftResult& expected)
{
  bool ok = true;
  ok = expect_eq(std::string(name) + ".nodes", actual.nodes, expected.nodes) && ok;
  ok = expect_eq(std::string(name) + ".captures", actual.captures, expected.captures) && ok;
  ok = expect_eq(std::string(name) + ".en_passant", actual.en_passant, expected.en_passant) && ok;
  ok = expect_eq(std::string(name) + ".castles", actual.castles, expected.castles) && ok;
  ok = expect_eq(std::string(name) + ".promotions", actual.promotions, expected.promotions) && ok;
  ok = expect_eq(std::string(name) + ".checks", actual.checks, expected.checks) && ok;
  return ok;
}

} // namespace

int main()
{
  Wyvern::Search search;
  bool ok = true;

  {
    Wyvern::Position position;
    ok = expect_perft("startpos.depth0", run_perft(search, position, 0), {1, 0, 0, 0, 0, 0}) && ok;
  }
  {
    Wyvern::Position position;
    ok = expect_perft("startpos.depth1", run_perft(search, position, 1), {20, 0, 0, 0, 0, 0}) && ok;
  }
  {
    Wyvern::Position position;
    ok =
      expect_perft("startpos.depth2", run_perft(search, position, 2), {400, 0, 0, 0, 0, 0}) && ok;
  }
  {
    Wyvern::Position position;
    ok = expect_perft("startpos.depth3", run_perft(search, position, 3), {8902, 34, 0, 0, 0, 12}) &&
         ok;
  }
  {
    Wyvern::Position position(kiwipete_fen);
    ok = expect_perft("kiwipete.depth1", run_perft(search, position, 1), {48, 8, 0, 2, 0, 0}) && ok;
  }
  {
    Wyvern::Position position(kiwipete_fen);
    ok =
      expect_perft("kiwipete.depth2", run_perft(search, position, 2), {2039, 351, 1, 91, 0, 3}) &&
      ok;
  }
  {
    Wyvern::TranspositionTable table(4);
    table.insert(0x1234ULL, Wyvern::BoundedEval(Wyvern::BOUND_LOWER, 42), 3);
    const Wyvern::BoundedEval stored = table.lookup(0x1234ULL, 3);
    ok = expect_bound("transposition.lower_bound", stored.bound, Wyvern::BOUND_LOWER) && ok;
    ok = expect_eq("transposition.lower_eval", stored.eval, 42) && ok;
  }

  return ok ? 0 : 1;
}
