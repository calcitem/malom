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

#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "PerfectPlayer.h"
#include "Player.h"
#include "main.h"
#include "move.h"
#include "MalomSolutionAccess.h"
#include "rules.h"

int MalomSolutionAccess::GetBestMove(int whiteBitboard, int blackBitboard, int whiteStonesToPlace, int blackStonesToPlace, int playerToMove, bool onlyStoneTaking)
{
    InitializeIfNeeded();

    GameState s;

    const int W = 0;
    const int B = 1;

    if ((whiteBitboard & blackBitboard) != 0) {
        throw std::invalid_argument("whiteBitboard and blackBitboard shouldn't have any overlap");
    }

    for (int i = 0; i < 24; i++) {
        if ((whiteBitboard & (1 << i)) != 0) {
            s.T[i] = W;
            s.StoneCount[W] += 1;
        }
        if ((blackBitboard & (1 << i)) != 0) {
            s.T[i] = B;
            s.StoneCount[B] += 1;
        }
    }

    s.phase = ((whiteStonesToPlace == 0 && blackStonesToPlace == 0) ? 2 : 1);
    MustBeBetween("whiteStonesToPlace", whiteStonesToPlace, 0, Rules::MaxKSZ);
    MustBeBetween("blackStonesToPlace", blackStonesToPlace, 0, Rules::MaxKSZ);
    s.SetStoneCount[W] = Rules::MaxKSZ - whiteStonesToPlace;
    s.SetStoneCount[B] = Rules::MaxKSZ - blackStonesToPlace;
    s.KLE = onlyStoneTaking;
    MustBeBetween("playerToMove", playerToMove, 0, 1);
    s.SideToMove = playerToMove;
    s.MoveCount = 10;

    if (s.FutureStoneCount[W] > Rules::MaxKSZ) {
        throw std::invalid_argument("Number of stones in whiteBitboard + whiteStonesToPlace > " + std::to_string(Rules::MaxKSZ));
    }
    if (s.FutureStoneCount[B] > Rules::MaxKSZ) {
        throw std::invalid_argument("Number of stones in blackBitboard + blackStonesToPlace > " + std::to_string(Rules::MaxKSZ));
    }

    std::string errorMsg = s.SetOverAndCheckValidSetup();
    if (errorMsg != "") {
        throw std::invalid_argument(errorMsg);
    }
    if (s.over) {
        throw std::invalid_argument("Game is already over.");
    }

    s.LastIrrev = 0;

    try {
        return pp->ChooseRandom(pp->GoodMoves(s)).ToBitBoard();
    } catch (std::out_of_range& e) {
        throw std::runtime_error("We don't have a database entry for this position. This can happen either if the database is corrupted (missing files), or sometimes when the position is not reachable from the starting position.");
    }
}

int MalomSolutionAccess::GetBestMoveNoException(int whiteBitboard, int blackBitboard, int whiteStonesToPlace, int blackStonesToPlace, int playerToMove, bool onlyStoneTaking)
{
    try {
        lastError = nullptr;
        return GetBestMove(whiteBitboard, blackBitboard, whiteStonesToPlace, blackStonesToPlace, playerToMove, onlyStoneTaking);
    } catch (std::exception& e) {
        lastError = &e;
        return 0;
    }
}

std::string MalomSolutionAccess::GetLastError()
{
    if (lastError == nullptr) {
        return "No error";
    }
    return lastError->what();
}

int MalomSolutionAccess::GetBestMoveStr(std::string args)
{
    try {
        std::istringstream iss(args);
        std::vector<std::string> argsSplit((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

        const int numArgs = 6;
        if (argsSplit.size() != numArgs)
            throw std::invalid_argument("Invalid number of arguments after splitting the string. Instead of " + std::to_string(numArgs) + ", got " + std::to_string(argsSplit.size()) + " arguments.");

        return GetBestMove(std::stoi(argsSplit[0]), std::stoi(argsSplit[1]), std::stoi(argsSplit[2]), std::stoi(argsSplit[3]), std::stoi(argsSplit[4]), argsSplit[5] != "0");
    } catch (std::exception& e) {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        return 0;
    }
}

void MalomSolutionAccess::InitializeIfNeeded()
{
    if (pp != nullptr) {
        return;
    }
    InitRules();
    SetVariantStripped();
    if (!Sectors::HasDatabase) {
        throw std::runtime_error("Database files not found in the current working directory (" + std::string(std::filesystem::current_path()) + ")");
    }
    pp = new PerfectPlayer();
}

void MalomSolutionAccess::MustBeBetween(std::string paramName, int value, int min, int max)
{
    if (value < min || value > max) {
        throw std::out_of_range(paramName + " must be between " + std::to_string(min) + " and " + std::to_string(max));
    }
}

void MalomSolutionAccess::SetVariantStripped()
{
    // copy-paste from Rules.cpp, but references to Main stripped

    switch (Wrappers::Constants::Variant) {
    case Wrappers::Constants::Variants::std:
        MillPos = StdLaskerMillPos;
        InvMillPos = StdLaskerInvMillPos;
        BoardGraph = StdLaskerBoardGraph;
        ALBoardGraph = StdLaskerALBoardGraph;
        MaxKSZ = 9;
        VariantName = "std";
        break;
    case Wrappers::Constants::Variants::lask:
        MillPos = StdLaskerMillPos;
        InvMillPos = StdLaskerInvMillPos;
        BoardGraph = StdLaskerBoardGraph;
        ALBoardGraph = StdLaskerALBoardGraph;
        MaxKSZ = 10;
        VariantName = "lask";
        break;
    case Wrappers::Constants::Variants::mora:
        MillPos = MoraMillPos;
        InvMillPos = MoraInvMillPos;
        BoardGraph = MoraBoardGraph;
        ALBoardGraph = MoraALBoardGraph;
        MaxKSZ = 12;
        VariantName = "mora";
        break;
    }

    if (Wrappers::Constants::Extended) {
        MaxKSZ = 12;
    }
}
