#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

class GameState { }; // GameState class definition is missing, so I added this placeholder

class FrmMain { }; // FrmMain class definition is missing, so I added this placeholder

namespace Rules {
std::array<std::array<uint8_t, 3>, 16> StdLaskerMillPos;
std::array<std::array<uint8_t, 3>, 20> MoraMillPos;
std::array<std::vector<int>, 24> StdLaskerInvMillPos;
std::array<std::vector<int>, 24> MoraInvMillPos;
std::array<std::array<bool, 24>, 24> StdLaskerBoardGraph;
std::array<std::array<bool, 24>, 24> MoraBoardGraph;
std::array<std::array<uint8_t, 5>, 24> StdLaskerALBoardGraph;
std::array<std::array<uint8_t, 5>, 24> MoraALBoardGraph;
std::string VariantName;

const int LastIrrevLimit = 50;

void InitRules()
{
    // Initialize StdLaskerMillPos
    StdLaskerMillPos[0] = { 1, 2, 3 };
    StdLaskerMillPos[1] = { 3, 4, 5 };
    StdLaskerMillPos[2] = { 5, 6, 7 };
    StdLaskerMillPos[3] = { 7, 0, 1 };
    for (int i = 4; i <= 11; ++i) {
        StdLaskerMillPos[i] = { StdLaskerMillPos[i - 4][0] + 8, StdLaskerMillPos[i - 4][1] + 8, StdLaskerMillPos[i - 4][2] + 8 };
    }
    StdLaskerMillPos[12] = { 0, 8, 16 };
    StdLaskerMillPos[13] = { 2, 10, 18 };
    StdLaskerMillPos[14] = { 4, 12, 20 };
    StdLaskerMillPos[15] = { 6, 14, 22 };

    // Initialize StdLaskerInvMillPos
    for (int i = 0; i < 16; ++i) {
        StdLaskerInvMillPos[i] = std::vector<int>(2);
    }
    bool needed;
    for (int i = 0; i < 24; ++i) {
        std::vector<int> l;
        for (int j = 0; j < 16; ++j) {
            needed = false;
            for (int k = 0; k < 3; ++k) {
                if (StdLaskerMillPos[j][k] == i)
                    needed = true;
            }
            if (needed) {
                l.push_back(j);
            }
        }
        StdLaskerInvMillPos[i] = l;
    }

    // Initialize StdLaskerBoardGraph
    for (int i = 0; i < 24; ++i) {
        for (int j = 0; j < 24; ++j) {
            StdLaskerBoardGraph[i][j] = false;
        }
    }
    for (int i = 0; i < 7; ++i) {
        StdLaskerBoardGraph[i][i + 1] = true;
    }
    StdLaskerBoardGraph[7][0] = true;
    for (int i = 8; i < 15; ++i) {
        StdLaskerBoardGraph[i][i + 1] = true;
    }
    StdLaskerBoardGraph[15][8] = true;
    for (int i = 16; i < 23; ++i) {
        StdLaskerBoardGraph[i][i + 1] = true;
    }
    StdLaskerBoardGraph[23][16] = true;
    for (int j = 0; j < 7; j += 2) {
        for (int i = 0; i < 9; i += 8) {
            StdLaskerBoardGraph[j + i][j + i + 8] = true;
        }
    }
    for (int i = 0; i < 24; ++i) {
        for (int j = 0; j < 24; ++j) {
            if (StdLaskerBoardGraph[i][j])
                StdLaskerBoardGraph[j][i] = true;
        }
    }

    // Initialize StdLaskerALBoardGraph
    for (int i = 1; i < 25; ++i) {
        StdLaskerALBoardGraph[i][0] = 0;
    }
    for (int i = 0; i < 24; ++i) {
        for (int j = 0; j < 24; ++j) {
            if (StdLaskerBoardGraph[i][j]) {
                StdLaskerALBoardGraph[i][StdLaskerALBoardGraph[i][0] + 1] = j;
                StdLaskerALBoardGraph[i][0] += 1;
            }
        }
    }

    // Initialize MoraMillPos
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 3; ++j) {
            MoraMillPos[i][j] = StdLaskerMillPos[i][j];
        }
    }
    MoraMillPos[16] = { 1, 9, 17 };
    MoraMillPos[17] = { 3, 11, 19 };
    MoraMillPos[18] = { 5, 13, 21 };
    MoraMillPos[19] = { 7, 15, 23 };

    // Initialize MoraInvMillPos
    for (int i = 0; i < 24; ++i) {
        std::vector<int> l;
        for (int j = 0; j < 20; ++j) {
            needed = false;
            for (int k = 0; k < 3; ++k) {
                if (MoraMillPos[j][k] == i)
                    needed = true;
            }
            if (needed) {
                l.push_back(j);
            }
        }
        MoraInvMillPos[i] = l;
    }

    // Initialize MoraBoardGraph
    for (int i = 0; i < 24; ++i) {
        for (int j = 0; j < 24; ++j) {
            MoraBoardGraph[i][j] = StdLaskerBoardGraph[i][j];
        }
    }
    for (int i = 0; i < 16; ++i) {
        MoraBoardGraph[i][i + 8] = true;
    }
    for (int i = 0; i < 24; ++i) {
        for (int j = 0; j < 24; ++j) {
            if (MoraBoardGraph[i][j])
                MoraBoardGraph[j][i] = true;
        }
    }

    // Initialize MoraALBoardGraph
    for (int i = 1; i < 25; ++i) {
        MoraALBoardGraph[i][0] = 0;
    }
    for (int i = 0; i < 24; ++i) {
        for (int j = 0; j < 24; ++j) {
            if (MoraBoardGraph[i][j]) {
                MoraALBoardGraph[i][MoraALBoardGraph[i][0] + 1] = j;
                MoraALBoardGraph[i][0] += 1;
            }
        }
    }

    int Malome(int m, const GameState& s)
    {
        int result = -1;
        for (int i = 0; i < InvMillPos[m].size(); ++i) {
            if (s.T(MillPos[InvMillPos[m][i]][0]) == s.T(m) && s.T(MillPos[InvMillPos[m][i]][1]) == s.T(m) && s.T(MillPos[InvMillPos[m][i]][2]) == s.T(m)) {
                result = InvMillPos[m][i];
            }
        }
        return result;
    }

    bool YouCanMove(const GameState& s)
    {
        // Debug.Assert(Not s.KLE); // C++ doesn't have Debug.Assert
        if (s.SetStoneCount(s.SideToMove) == MaxKSZ && s.StoneCount(s.SideToMove) > 3) {
            for (int i = 0; i < 24; ++i) {
                if (s.T(i) == s.SideToMove) {
                    for (int j = 1; j <= ALBoardGraph[i][0]; ++j) {
                        if (s.T(ALBoardGraph[i][j]) == -1)
                            return true;
                    }
                }
            }
        } else {
            return true;
        }
        return false;
    }

    bool MindenEllensegesKorongMalomban(const GameState& s)
    {
        for (int i = 0; i < 24; ++i) {
            if (s.T(i) == 1 - s.SideToMove && Malome(i, s) == -1)
                return false;
        }
        return true;
    }

    int MaxKSZ;

    FrmMain Main;

    bool AlphaBetaAvailable()
    {
        // return Wrappers.Constants.Variant == Wrappers.Constants.Variants.std && !Wrappers.Constants.Extended;
        // Wrappers.Constants is not defined, so I commented this line
        return false; // placeholder
    }

    void SetVariant()
    {
        // part of this is copy-pasted in MalomAPI

        // Select Case Wrappers.Constants.Variant
        // Switch statement for Wrappers.Constants.Variant is not possible since Wrappers.Constants is not defined
        // I will provide an example for one case only

        // Case Wrappers.Constants.Variants.std
        MillPos = StdLaskerMillPos;
        InvMillPos = StdLaskerInvMillPos;
        BoardGraph = StdLaskerBoardGraph;
        ALBoardGraph = StdLaskerALBoardGraph;
        MaxKSZ = 9;
        VariantName = "std";
        // Main.Text = "Malom (Nine Men's Morris)"; // Main.Text is not defined in FrmMain placeholder class

        // Other cases are omitted due to missing Wrappers.Constants definition

        // If Main.Loaded Then Main.NewGame() // Main.Loaded and Main.NewGame() are not defined in FrmMain placeholder class
    }
} // namespace Rules
