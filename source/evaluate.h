#ifndef _EVALUATE_H_
#define _EVALUATE_H_

#include "types.h"
#include "position.h"

namespace Eval
{
	// Apery(WCSC26)の駒割り
	enum {
		PawnValue = 90,
		SilverValue = 495,
		GoldValue = 540,
		BishopValue = 855,
		RookValue = 990,
		ProPawnValue = 540,
		ProSilverValue = 540,
		HorseValue = 945,
		DragonValue = 1395,
		KingValue = 15000,
	};

	// 駒の価値のテーブル(後手の駒は負の値)
	extern int PieceValue[PIECE_NB];

  Value evaluate(const Position& pos);
}

#endif