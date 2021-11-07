#include <algorithm>
#include <thread>

#include "search.h"
#include "usi.h"
#include "evaluate.h"
#include "misc.h"

using namespace std;

namespace Search
{
    // 探索開始局面で思考対象とする指し手の集合。
    RootMoves rootMoves;

    // 持ち時間設定など。
    LimitsType Limits;

    // 今回のgoコマンドでの探索ノード数。
    uint64_t Nodes;

    // 探索中にこれがtrueになったら探索を即座に終了すること。
    bool Stop;
}

// 起動時に呼び出される。時間のかからない探索関係の初期化処理はここに書くこと。
void Search::init() {}

// isreadyコマンドの応答中に呼び出される。時間のかかる処理はここに書くこと。
void Search::clear() {}

// 探索を開始する
void Search::start_thinking(const Position& rootPos, StateListPtr& states, LimitsType limits)
{
    Limits = limits;
    rootMoves.clear();
    Nodes = 0;
    Stop = false;

    for (Move move : MoveList<LEGAL_ALL>(rootPos))
        rootMoves.emplace_back(move);

    ASSERT_LV3(states.get());
    
    Position* pos_ptr = const_cast<Position*>(&rootPos);
    search(*pos_ptr);
}

// 探索本体
void Search::search(Position& pos)
{
    // 探索で返す指し手
    Move bestMove = MOVE_RESIGN;

    if (rootMoves.size() == 0)
    {
        // 合法手が存在しない
        Stop = true;
        goto END;
    }

    /* ここから探索部を記述する */



    /* 探索部をここまで */

END:;
    cout << "bestmove " << bestMove << endl;
}