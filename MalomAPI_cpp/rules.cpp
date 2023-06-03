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

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "MalomSolutionAccess.h"
#include "PerfectPlayer.h"
#include "Player.h"
#include "game_state.h"
#include "move.h"
#include "rules.h"

void Rules::initRules()
{
    stdLaskerMillPos[0][0] = 1;
    stdLaskerMillPos[0][1] = 2;
    stdLaskerMillPos[0][2] = 3;
    stdLaskerMillPos[1][0] = 3;
    stdLaskerMillPos[1][1] = 4;
    stdLaskerMillPos[1][2] = 5;
    stdLaskerMillPos[2][0] = 5;
    stdLaskerMillPos[2][1] = 6;
    stdLaskerMillPos[2][2] = 7;
    stdLaskerMillPos[3][0] = 7;
    stdLaskerMillPos[3][1] = 0;
    stdLaskerMillPos[3][2] = 1;
    for (int i = 4; i <= 11; i++) {
        stdLaskerMillPos[i][0] = stdLaskerMillPos[i - 4][0] + 8;
        stdLaskerMillPos[i][1] = stdLaskerMillPos[i - 4][1] + 8;
        stdLaskerMillPos[i][2] = stdLaskerMillPos[i - 4][2] + 8;
    }
    stdLaskerMillPos[12][0] = 0;
    stdLaskerMillPos[13][0] = 2;
    stdLaskerMillPos[14][0] = 4;
    stdLaskerMillPos[15][0] = 6;
    for (int i = 12; i <= 15; i++) {
        stdLaskerMillPos[i][1] = stdLaskerMillPos[i][0] + 8;
        stdLaskerMillPos[i][2] = stdLaskerMillPos[i][0] + 16;
    }
    // Since C++ arrays cannot be resized dynamically, we'll need to allocate memory for stdLaskerInvMillPos beforehand, and then populate it in this function.
    bool kell;
    for (int i = 0; i <= 23; i++) {
        std::vector<int> l;
        for (int j = 0; j <= 15; j++) {
            kell = false;
            for (int k = 0; k <= 2; k++) {
                if (stdLaskerMillPos[j][k] == i)
                    kell = true;
            }
            if (kell) {
                l.push_back(j);
            }
        }
        // Convert the vector into an array and store it in stdLaskerInvMillPos
        stdLaskerInvMillPos[i] = new int[l.size()];
        for (int j = 0; j < l.size(); j++) {
            stdLaskerInvMillPos[i][j] = l[j];
        }
    }
    // Initialize stdLaskerBoardGraph with false
    for (int i = 0; i <= 23; i++) {
        for (int j = 0; j <= 23; j++) {
            stdLaskerBoardGraph[i][j] = false;
        }
    }
    // Fill the board
    for (int i = 0; i <= 6; i++) {
        stdLaskerBoardGraph[i][i + 1] = true;
    }
    stdLaskerBoardGraph[7][0] = true;
    for (int i = 8; i <= 14; i++) {
        stdLaskerBoardGraph[i][i + 1] = true;
    }
    stdLaskerBoardGraph[15][8] = true;
    for (int i = 16; i <= 22; i++) {
        stdLaskerBoardGraph[i][i + 1] = true;
    }
    stdLaskerBoardGraph[23][16] = true;
    for (int j = 0; j <= 6; j += 2) {
        for (int i = 0; i <= 8; i += 8) {
            stdLaskerBoardGraph[j + i][j + i + 8] = true;
        }
    }
    // Fill the rest of the graph
    for (int i = 0; i <= 23; i++) {
        for (int j = 0; j <= 23; j++) {
            if (stdLaskerBoardGraph[i][j] == true) {
                stdLaskerBoardGraph[j][i] = true;
            }
        }
    }
    // Initialize stdLaskerALBoardGraph with 0
    for (int i = 0; i <= 23; i++) {
        stdLaskerALBoardGraph[i][0] = 0;
    }
    // Fill the rest of the graph
    for (int i = 0; i <= 23; i++) {
        for (int j = 0; j <= 23; j++) {
            if (stdLaskerBoardGraph[i][j] == true) {
                stdLaskerALBoardGraph[i][stdLaskerALBoardGraph[i][0] + 1] = j;
                stdLaskerALBoardGraph[i][0] += 1;
            }
        }
    }
}

// Returns -1 if there is no mill on the given field, otherwise returns the sequence number in StdLaskerMalomPoz
int Rules::malome(int m, GameState s)
{
    int result = -1;
    for (int i = 0; i < invMillPos[m].size(); i++) {
        if (s.T[millPos[invMillPos[m][i]][0]] == s.T[m] && s.T[millPos[invMillPos[m][i]][1]] == s.T[m] && s.T[millPos[invMillPos[m][i]][2]] == s.T[m]) {
            result = invMillPos[m][i];
        }
    }
    return result;
}

// Tells whether the next player can move '(doesn't handle the kle case)
bool Rules::youCanMove(const GameState &s)
{
    assert(!s.kle);
    if (s.setStoneCount[s.sideToMove] == maxKSZ && s.stoneCount[s.sideToMove] > 3) {
        for (int i = 0; i <= 23; i++) {
            if (s.T[i] == s.sideToMove) {
                for (int j = 1; j <= aLBoardGraph[i][0]; j++) {
                    if (s.T[aLBoardGraph[i][j]] == -1)
                        return true;
                }
            }
        }
    } else {
        return true;
    }
    return false;
}

bool Rules::mindenEllensegesKorongMalomban(GameState s)
{
    for (int i = 0; i <= 23; i++) {
        if (s.T[i] == 1 - s.sideToMove && malome(i, s) == -1)
            return false;
    }
    return true;
}

int maxKSZ;

// Checking if AlphaBeta is available
bool Rules::alphaBetaAvailable()
{
    return Wrappers::Constants::variant == (int)Wrappers::Constants::Variants::std && !Wrappers::Constants::extended;
}

void Rules::setVariant()
{
    // Part of this is copy-pasted in MalomAPI
    if (Wrappers::Constants::variant == (int)Wrappers::Constants::Variants::std) {
        std::memcpy(millPos, stdLaskerMillPos, sizeof(stdLaskerMillPos));
        for (int i = 0; i < 24; ++i) {
            invMillPos[i] = stdLaskerInvMillPos[i];
        }
        std::memcpy(boardGraph, stdLaskerBoardGraph, sizeof(stdLaskerBoardGraph));
        std::memcpy(aLBoardGraph, stdLaskerALBoardGraph, sizeof(stdLaskerALBoardGraph));
        maxKSZ = 9;
        variantName = "std";
    } else if (Wrappers::Constants::variant == (int)Wrappers::Constants::Variants::lask) {
        std::memcpy(millPos, stdLaskerMillPos, sizeof(stdLaskerMillPos));
        for (int i = 0; i < 24; ++i) {
            invMillPos[i] = stdLaskerInvMillPos[i];
        }
        std::memcpy(boardGraph, stdLaskerBoardGraph, sizeof(stdLaskerBoardGraph));
        std::memcpy(aLBoardGraph, stdLaskerALBoardGraph, sizeof(stdLaskerALBoardGraph));
        maxKSZ = 10;
        variantName = "lask";
    } else if (Wrappers::Constants::variant == (int)Wrappers::Constants::Variants::mora) {
        std::memcpy(millPos, moraMillPos, sizeof(moraMillPos));
        for (int i = 0; i < 24; ++i) {
            invMillPos[i] = moraInvMillPos[i];
        }
        std::memcpy(boardGraph, moraBoardGraph, sizeof(moraBoardGraph));
        std::memcpy(aLBoardGraph, moraALBoardGraph, sizeof(moraALBoardGraph));
        maxKSZ = 12;
        variantName = "mora";
    }

    if (Wrappers::Constants::extended) {
        maxKSZ = 12;
    }
}
