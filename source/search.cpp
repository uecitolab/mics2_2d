#include <algorithm>
#include <thread>

#include "search.h"
#include "usi.h"
#include "evaluate.h"
#include "misc.h"

struct MovePicker
{
    // 静止探索で使用
    MovePicker(const Position& pos_, Square recapSq) : pos(pos_)
    {
        if (pos.in_check())
            endMoves = generateMoves<EVASIONS>(pos, currentMoves);
        else
            endMoves = generateMoves<RECAPTURES>(pos, currentMoves, recapSq);
    }

    Move nextMove() 
    {
        if (currentMoves == endMoves)
            return MOVE_NONE;
        return *currentMoves++;
    }

private:
    const Position& pos;
    ExtMove moves[MAX_MOVES], * currentMoves = moves, * endMoves = moves;
};

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

Value search(Position& pos, Value alpha, Value beta, int depth, int ply_from_root);
Value qsearch(Position& pos, Value alpha, Value beta, int depth, int ply_from_root);

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
    {
        /* 時間制御 */
        Color us = pos.side_to_move();
        std::thread* timerThread = nullptr;

        // 今回は秒読み以外の設定は考慮しない
        s64 endTime = Limits.byoyomi[us] - 150;

        timerThread = new std::thread([&] {
            while (Time.elapsed() < endTime && !Stop)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            Stop = true;
        });

        // 探索深さ．初期値は1
        int rootDepth = 1;

        // 反復深化
        while (!Stop)
        {
            // α値
            Value alpha = -VALUE_INFINITE;

            // β値
            Value beta = VALUE_INFINITE;

            // do_move()に必要
            StateInfo si;

            for (int i = 0; i < rootMoves.size(); ++i)
            {
                Move move = rootMoves[i].pv[0];

                // 局面を1手進める
                pos.do_move(move, si);

                // search()を呼び出す
                Value value = -search(pos, alpha, beta, rootDepth - 1, 0);

                // 探索終了であれば返り値は信用できない
                if (Stop)
                    break;

                // 局面を1手戻す
                pos.undo_move(move);

                if (value > alpha)
                    alpha = value;

                rootMoves[i].score = value;
            }

            // 合法手を評価値の高い順に並び替える
            std::stable_sort(rootMoves.begin(), rootMoves.end());

            // インクリメント
            ++rootDepth;
        }

        // プレイヤが返す指し手
        bestMove = rootMoves[0].pv[0];

        // タイマースレッド終了
        Stop = true;
        if (timerThread != nullptr)
        {
            timerThread->join();
            delete timerThread;
        }
    }
    /* 探索部ここまで */

END:;
    std::cout << "bestmove " << bestMove << std::endl;
}

Value search(Position& pos, Value alpha, Value beta, int depth, int ply_from_root)
{
    if (depth <= 0)
        return qsearch(pos, alpha, beta, depth, ply_from_root);

    StateInfo si;

    // この局面でdo_move()された合法手の数
    int moveCount = 0;

    // 千日手の検出
    RepetitionState draw_type = pos.is_repetition();
    if (draw_type != REPETITION_NONE)
        return draw_value(draw_type, pos.side_to_move());

    for (ExtMove m : MoveList<LEGAL>(pos))
    {
        // 局面を1手進める
        pos.do_move(m.move, si);
        ++moveCount;

        // 再帰的にsearch()を呼び出す
        Value value = -search(pos, -beta, -alpha, depth - 1, ply_from_root + 1);

        // 局面を1手戻す
        pos.undo_move(m.move);

        // 探索の終了
        if (Search::Stop)
            return VALUE_ZERO;

        if (value > alpha)
        {
            alpha = value;
            if (alpha >= beta)
                break;
        }
    }

    if (moveCount == 0)
        return mated_in(ply_from_root);

    return alpha;
}

// 静止探索
Value qsearch(Position& pos, Value alpha, Value beta, int depth, int ply_from_root)
{
    // この局面で王手がかかっているか
    bool InCheck = pos.in_check();

    Value value;
    if (InCheck)
    {
        // 王手がかかっているならすべての指し手を調べる
        alpha = -VALUE_INFINITE;
    }
    else
    {
        // この局面で何も指さないときの評価値
        // ここで評価関数を呼び出す
        value = Eval::evaluate(pos);

        if (alpha < value)
        {
            alpha = value;
            if (alpha >= beta)
                return alpha; // 枝刈り
        }

        // これ以上探索を延長しない
        // ここでは適当に-3とした
        if (depth < -3)
            return alpha;
    }

    // do_move()で必要
    StateInfo si;

    // この局面でdo_move()された合法手の数
    int move_count = 0;

    // 千日手の検出
    RepetitionState draw_type = pos.is_repetition();
    if (draw_type != REPETITION_NONE)
        return draw_value(draw_type, pos.side_to_move());

    MovePicker mp(pos, move_to(pos.state()->lastMove));
    Move move;
    while ((move = mp.nextMove()) != MOVE_NONE)
    {
        // LEGAL, LEAGL_ALL以外で指し手生成しているため
        // 合法かを確認する
        if (!pos.legal(move))
            continue;

        // 局面を1手進める
        pos.do_move(move, si);
        ++move_count;

        // 再帰的にqsearch()を呼び出す
        Value value = -qsearch(pos, -beta, -alpha, depth - 1, ply_from_root + 1);

        // 局面を1手戻す
        pos.undo_move(move);

        // 探索の終了
        if (Search::Stop)
            return VALUE_ZERO;

        if (value > alpha)
        {
            alpha = value;
            if (alpha >= beta)
                break;
        }
    }

    // 王手されていて，do_move()した数が0なら詰んでいる
    // 詰みのスコアを返す
    if (InCheck && move_count == 0)
        return mated_in(ply_from_root);

    return alpha;
}