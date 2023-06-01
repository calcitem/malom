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

#ifndef RULES_H_INCLUDED
#define RULES_H_INCLUDED

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "PerfectPlayer.h"
#include "Player.h"
#include "main.h"
#include "move.h"

#include "wrappers.h"

// Define byte as unsigned char
typedef unsigned char byte;

class Rules {
public:
    // Define your byte arrays
    byte millPos[16][3];
    byte stdLaskerMillPos[16][3];
    byte moraMillPos[20][3];

    // Define your integer arrays
    int* invMillPos[24];
    int* stdLaskerInvMillPos[24];
    int* moraInvMillPos[24];

    // Define your boolean arrays
    bool boardGraph[24][24];
    bool stdLaskerBoardGraph[24][24];
    bool moraBoardGraph[24][24];

    // Define your adjacency list byte arrays
    byte aLBoardGraph[24][5];
    byte stdLaskerALBoardGraph[24][5];
    byte moraALBoardGraph[24][5];

    // Define other variables
    static std::string variantName;
    static int maxKSZ;
    static const int lastIrrevLimit = 50;

public:
    // Add your public methods here

    void InitRules();

    // Returns -1 if there is no mill on the given field, otherwise returns the sequence number in StdLaskerMalomPoz
    int Malome(int m, GameState s);

    // Tells whether the next player can move '(doesn't handle the KLE case)
    bool YouCanMove(GameState s);

    bool MindenEllensegesKorongMalomban(GameState s);

    // Checking if AlphaBeta is available
    bool AlphaBetaAvailable()
    {
        return Wrappers::Constants::Variant == Wrappers::Constants::Variants::std && !Wrappers::Constants::Extended;
    }

    void SetVariant();
};

#endif // RULES_H_INCLUDED