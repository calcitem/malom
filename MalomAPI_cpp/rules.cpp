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

// Define byte as unsigned char
typedef unsigned char byte;

class Rules {
public:
    // Define your byte arrays
    byte MillPos[16][3];
    byte StdLaskerMillPos[16][3];
    byte MoraMillPos[20][3];

    // Define your integer arrays
    int* InvMillPos[24];
    int* StdLaskerInvMillPos[24];
    int* MoraInvMillPos[24];

    // Define your boolean arrays
    bool BoardGraph[24][24];
    bool StdLaskerBoardGraph[24][24];
    bool MoraBoardGraph[24][24];

    // Define your adjacency list byte arrays
    byte ALBoardGraph[24][5];
    byte StdLaskerALBoardGraph[24][5];
    byte MoraALBoardGraph[24][5];

    // Define other variables
    std::string VariantName;
    int MaxKSZ;
    const int LastIrrevLimit = 50;

public:
    // Add your public methods here

void InitRules()
    {
        StdLaskerMillPos[0][0] = 1;
        StdLaskerMillPos[0][1] = 2;
        StdLaskerMillPos[0][2] = 3;
        StdLaskerMillPos[1][0] = 3;
        StdLaskerMillPos[1][1] = 4;
        StdLaskerMillPos[1][2] = 5;
        StdLaskerMillPos[2][0] = 5;
        StdLaskerMillPos[2][1] = 6;
        StdLaskerMillPos[2][2] = 7;
        StdLaskerMillPos[3][0] = 7;
        StdLaskerMillPos[3][1] = 0;
        StdLaskerMillPos[3][2] = 1;
        for (int i = 4; i <= 11; i++) {
            StdLaskerMillPos[i][0] = StdLaskerMillPos[i - 4][0] + 8;
            StdLaskerMillPos[i][1] = StdLaskerMillPos[i - 4][1] + 8;
            StdLaskerMillPos[i][2] = StdLaskerMillPos[i - 4][2] + 8;
        }
        StdLaskerMillPos[12][0] = 0;
        StdLaskerMillPos[13][0] = 2;
        StdLaskerMillPos[14][0] = 4;
        StdLaskerMillPos[15][0] = 6;
        for (int i = 12; i <= 15; i++) {
            StdLaskerMillPos[i][1] = StdLaskerMillPos[i][0] + 8;
            StdLaskerMillPos[i][2] = StdLaskerMillPos[i][0] + 16;
        }
        // Since C++ arrays cannot be resized dynamically, we'll need to allocate memory for StdLaskerInvMillPos beforehand, and then populate it in this function.
        bool kell;
        for (int i = 0; i <= 23; i++) {
            std::vector<int> l;
            for (int j = 0; j <= 15; j++) {
                kell = false;
                for (int k = 0; k <= 2; k++) {
                    if (StdLaskerMillPos[j][k] == i)
                        kell = true;
                }
                if (kell) {
                    l.push_back(j);
                }
            }
            // Convert the vector into an array and store it in StdLaskerInvMillPos
            StdLaskerInvMillPos[i] = new int[l.size()];
            for (int j = 0; j < l.size(); j++) {
                StdLaskerInvMillPos[i][j] = l[j];
            }
        }
        // Initialize StdLaskerBoardGraph with false
        for (int i = 0; i <= 23; i++) {
            for (int j = 0; j <= 23; j++) {
                StdLaskerBoardGraph[i][j] = false;
            }
        }
        // Fill the board
        for (int i = 0; i <= 6; i++) {
            StdLaskerBoardGraph[i][i + 1] = true;
        }
        StdLaskerBoardGraph[7][0] = true;
        for (int i = 8; i <= 14; i++) {
            StdLaskerBoardGraph[i][i + 1] = true;
        }
        StdLaskerBoardGraph[15][8] = true;
        for (int i = 16; i <= 22; i++) {
            StdLaskerBoardGraph[i][i + 1] = true;
        }
        StdLaskerBoardGraph[23][16] = true;
        for (int j = 0; j <= 6; j += 2) {
            for (int i = 0; i <= 8; i += 8) {
                StdLaskerBoardGraph[j + i][j + i + 8] = true;
            }
        }
        // Fill the rest of the graph
        for (int i = 0; i <= 23; i++) {
            for (int j = 0; j <= 23; j++) {
                if (StdLaskerBoardGraph[i][j] == true) {
                    StdLaskerBoardGraph[j][i] = true;
                }
            }
        }
        // Initialize StdLaskerALBoardGraph with 0
        for (int i = 0; i <= 23; i++) {
            StdLaskerALBoardGraph[i][0] = 0;
        }
        // Fill the rest of the graph
        for (int i = 0; i <= 23; i++) {
            for (int j = 0; j <= 23; j++) {
                if (StdLaskerBoardGraph[i][j] == true) {
                    StdLaskerALBoardGraph[i][StdLaskerALBoardGraph[i][0] + 1] = j;
                    StdLaskerALBoardGraph[i][0] += 1;
                }
            }
        }
    }

    // Returns -1 if there is no mill on the given field, otherwise returns the sequence number in StdLaskerMalomPoz
    int Malome(int m, GameState s)
    {
        int result = -1;
        for (int i = 0; i < InvMillPos[m].size(); i++) {
            if (s.T[MillPos[InvMillPos[m][i]][0]] == s.T[m] && s.T[MillPos[InvMillPos[m][i]][1]] == s.T[m] && s.T[MillPos[InvMillPos[m][i]][2]] == s.T[m]) {
                result = InvMillPos[m][i];
            }
        }
        return result;
    }

    // Tells whether the next player can move '(doesn't handle the KLE case)
    bool YouCanMove(GameState s)
    {
        assert(!s.KLE);
        if (s.SetStoneCount(s.SideToMove) == MaxKSZ && s.StoneCount(s.SideToMove) > 3) {
            for (int i = 0; i <= 23; i++) {
                if (s.T[i] == s.SideToMove) {
                    for (int j = 1; j <= ALBoardGraph[i][0]; j++) {
                        if (s.T[ALBoardGraph[i][j]] == -1)
                            return true;
                    }
                }
            }
        } else {
            return true;
        }
        return false;
    }

    bool MindenEllensegesKorongMalomban(GameState s)
    {
        for (int i = 0; i <= 23; i++) {
            if (s.T[i] == 1 - s.SideToMove && Malome(i, s) == -1)
                return false;
        }
        return true;
    }

        int MaxKSZ;

    // Checking if AlphaBeta is available
    bool AlphaBetaAvailable()
    {
        return Wrappers.Constants.Variant == Wrappers.Constants.Variants.std && !Wrappers.Constants.Extended;
    }

    void SetVariant()
    {
        // Part of this is copy-pasted in MalomAPI
        if (Wrappers.Constants.Variant == Wrappers.Constants.Variants.std) {
            MillPos = StdLaskerMillPos;
            InvMillPos = StdLaskerInvMillPos;
            BoardGraph = StdLaskerBoardGraph;
            ALBoardGraph = StdLaskerALBoardGraph;
            MaxKSZ = 9;
            VariantName = "std";
        } else if (Wrappers.Constants.Variant == Wrappers.Constants.Variants.lask) {
            MillPos = StdLaskerMillPos;
            InvMillPos = StdLaskerInvMillPos;
            BoardGraph = StdLaskerBoardGraph;
            ALBoardGraph = StdLaskerALBoardGraph;
            MaxKSZ = 10;
            VariantName = "lask";
        } else if (Wrappers.Constants.Variant == Wrappers.Constants.Variants.mora) {
            MillPos = MoraMillPos;
            InvMillPos = MoraInvMillPos;
            BoardGraph = MoraBoardGraph;
            ALBoardGraph = MoraALBoardGraph;
            MaxKSZ = 12;
            VariantName = "mora";
        }

        if (Wrappers.Constants.Extended) {
            MaxKSZ = 12;
        }
    }
};
