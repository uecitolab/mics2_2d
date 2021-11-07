#include "evaluate.h"

namespace Eval
{
  int PieceValue[PIECE_NB] =
  {
    0, PawnValue, 0, 0, SilverValue, BishopValue, RookValue,GoldValue,
    KingValue, ProPawnValue, 0, 0, ProSilverValue, HorseValue, DragonValue,0,

    0, -PawnValue, 0, 0, -SilverValue, -BishopValue, -RookValue,-GoldValue,
    -KingValue, -ProPawnValue, 0, 0, -ProSilverValue, -HorseValue, -DragonValue,0,
  };

  Value evaluate(const Position& pos)
  {
    auto score = VALUE_ZERO;

    for (Square sq : SQ)
      score += PieceValue[pos.piece_on(sq)];

    for (Color c : COLOR)
      for (Piece pc : { PAWN,SILVER,BISHOP,ROOK,GOLD })
        score += (c == BLACK ? 1 : -1) * Value(hand_count(pos.hand_of(c), pc) * PieceValue[pc]);

    // 手番側から見た評価値を返す
    return pos.side_to_move() == BLACK ? score : -score;
  }
}
