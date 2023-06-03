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

#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <bitset>
#include <vector>
#include <cassert> // for assert
#include <cstdint> // for int64_t
#include <cstdlib> // for std::exit
#include <exception> // for std::exception
#include <iostream> // for std::cerr
#include <mutex> // for std::mutex and std::lock_guard
#include <stdexcept> // for std::out_of_range
#include <functional>

#include "Player.h"
//#include "main.h"
#include "move.h"
#include "rules.h"
#include "sector.h"
#include "common.h"
#include "Wrappers.h"

enum class MoveType {
    SetMove,
    SlideMove // should be renamed to SlideOrJumpMove
};

struct ExtMove {
    int hon, hov;
    MoveType moveType;
    bool withTaking, onlyTaking; // withTaking includes the steps in mill closure, onlyTaking only includes removal
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

class Sectors {
public:
    static std::map<id, Sector> sectors;
    static bool created;

    static std::map<id, Sector> getsectors();

    static bool hasDatabase();
};

// Initialize static member variables
std::map<id, Sector> Sectors::sectors;
bool Sectors::created = false;

class PerfectPlayer : public Player {
public:
    std::map<id, Sector> secs;

    PerfectPlayer();

    void enter(Game *_g);

    void quit() override
    {
        Player::quit();
    }

    WSector* getSec(GameState s);

    std::string toHumanReadableEval(Wrappers::gui_eval_elem2 e);

    int futureKorongCount(GameState& s);

    bool makesMill(GameState& s, int hon, int hov);

    bool isMill(GameState& s, int m);

    std::vector<ExtMove> setMoves(GameState& s);

    std::vector<ExtMove> slideMoves(GameState& s);

    // m has a withTaking step, where takeHon is not filled out. This function creates a list, the elements of which are copies of m supplemented with one possible removal each.
    std::vector<ExtMove> withTakingMoves(GameState& s, ExtMove& m);

    std::vector<ExtMove> onlyTakingMoves(GameState& s);

    std::vector<ExtMove> getMoveList(GameState& s);

    GameState makeMoveInState(GameState& s, ExtMove& m);

    // Assuming gui_eval_elem2 and getSec functions are defined somewhere
    Wrappers::gui_eval_elem2 moveValue(GameState& s, ExtMove& m);

    template <typename T, typename K>
    std::vector<T> allMaxBy(std::function<K(T)> f, std::vector<T>& l, K minValue);

    // Assuming the definition of gui_eval_elem2::min_value function
    std::vector<ExtMove> goodMoves(GameState& s);

    int NGMAfterMove(GameState& s, ExtMove& m);

    template <typename T>
    T chooseRandom(const std::vector<T>& l);

    void sendMoveToGUI(ExtMove m);

    void toMove(GameState& s);

    int numGoodMoves(GameState& s);

    int cp;

    struct MoveValuePair {
        ExtMove m;
        double val;
    };

    const double WRGMInf = 2; // Is this good?

    std::mutex evalLock;

    Wrappers::gui_eval_elem2 eval(GameState s);

    int64_t boardNegate(int64_t a);
};

#endif // PERFECT_PLAYER_H_INCLUDED
