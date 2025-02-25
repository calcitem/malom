// Malom, a Nine Men's Morris (and variants) player and solver program.
// Copyright(C) 2007-2016  Gabor E. Gevay, Gabor Danner
// Copyright (C) 2023 The Sanmill developers (see AUTHORS file)
//
// See our webpage (and the paper linked from there):
// http://compalg.inf.elte.hu/~ggevay/mills/index.php
//
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef PERFECT_PLAYER_H_INCLUDED
#define PERFECT_PLAYER_H_INCLUDED

#include <bitset>
#include <cassert>   // for assert
#include <cstdint>   // for int64_t
#include <cstdlib>   // for std::exit
#include <exception> // for std::exception
#include <fstream>
#include <functional>
#include <iostream>
#include <iostream> // for std::cerr
#include <map>
#include <mutex> // for std::mutex and std::lock_guard
#include <stdexcept>
#include <stdexcept> // for std::out_of_range
#include <string>
#include <vector>

#include "Player.h"
//#include "main.h"
#include "common.h"
#include "move.h"
#include "rules.h"
#include "sector.h"
#include "wrappers.h"

enum class MoveType {
    SetMove,
    SlideMove // should be renamed to SlideOrJumpMove
};

struct ExtMove
{
    int hon, hov;
    MoveType moveType;
    bool withTaking, onlyTaking; // withTaking includes the steps in mill
                                 // closure, onlyTaking only includes removal
    int takeHon;

    int toBitBoard()
    {
        if (onlyTaking) {
            return 1 << takeHon;
        }
        int ret = 1 << hov;
        if (moveType == MoveType::SlideMove) {
            ret += 1 << hon;
        }
        if (withTaking) {
            ret += 1 << takeHon;
        }
        return ret;
    }
};

class Sectors
{
public:
    static std::map<Wrappers::WID, Wrappers::WSector> sectors;
    static bool created;

    static std::map<Wrappers::WID, Wrappers::WSector> getSectors();

    static bool hasDatabase();
};

class PerfectPlayer : public Player
{
public:
    std::map<Wrappers::WID, Wrappers::WSector> secs;

    PerfectPlayer();

    void enter(Game *_g) override;

    void quit() override { Player::quit(); }

    Wrappers::WSector *getSec(const GameState s);

    std::string toHumanReadableEval(Wrappers::gui_eval_elem2 e);

    int futureKorongCount(const GameState &s);

    bool makesMill(const GameState &s, int hon, int hov);

    bool isMill(const GameState &s, int m);

    std::vector<ExtMove> setMoves(const GameState &s);

    std::vector<ExtMove> slideMoves(const GameState &s);

    // m has a withTaking step, where takeHon is not filled out. This function
    // creates a list, the elements of which are copies of m supplemented with
    // one possible removal each.
    std::vector<ExtMove> withTakingMoves(const GameState &s, ExtMove &m);

    std::vector<ExtMove> onlyTakingMoves(const GameState &s);

    std::vector<ExtMove> getMoveList(const GameState &s);

    GameState makeMoveInState(const GameState &s, ExtMove &m);

    // Assuming gui_eval_elem2 and getSec functions are defined somewhere
    Wrappers::gui_eval_elem2 moveValue(const GameState &s, ExtMove &m);

    template <typename T, typename K>
    std::vector<T> allMaxBy(std::function<K(T)> f, const std::vector<T> &l,
                            K minValue);

    // Assuming the definition of gui_eval_elem2::min_value function
    std::vector<ExtMove> goodMoves(const GameState &s);

    int NGMAfterMove(const GameState &s, ExtMove &m);

    template <typename T>
    T chooseRandom(const std::vector<T> &l);

    void sendMoveToGUI(ExtMove m);

    void toMove(const GameState &s) override;

    int numGoodMoves(const GameState &s);

    int cp;

    struct MoveValuePair
    {
        ExtMove m;
        double val;
    };

    // const double WRGMInf = 2; // Is this good?

    std::mutex evalLock;

    Wrappers::gui_eval_elem2 eval(GameState s);

    int64_t boardNegate(int64_t a);
};

#endif // PERFECT_PLAYER_H_INCLUDED
