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

#include <list>
#include <stdexcept>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <cassert>
#include <string>

class Player; // forward declaration, implement this
class GameState; // forward declaration, implement this
class Move; // forward declaration, implement this

class Game {
private:
    Player* _Ply[2]; // players in the game
    std::list<GameState> history; // GameStates in this (and previous) games
    std::list<GameState>::iterator current; // the node of the current GameState in history

public:
    const GameState& s() const
    { // wrapper of current.value
        return *current;
    }

    Game(Player* p1, Player* p2)
    {
        history.push_back(GameState());
        current = std::prev(history.end());
        _Ply[0] = p1;
        _Ply[1] = p2;
    }

    Player** Plys()
    {
        return _Ply;
    }

    Player* Ply(int i) const
    { // get players in the game
        return _Ply[i];
    }

    void set_Ply(int i, Player* p)
    { // set players in the game
        if (p == nullptr) {
            _Ply[i] = nullptr;
            return;
        }

        p->Quit(); // we exit p to see if it was in a game (e.g. NewGame in the previous one)
        if (_Ply[i] != nullptr)
            _Ply[i]->Quit(); // the player replaced by p is kicked out
        _Ply[i] = p;
        p->Enter(this);
    }

    void MakeMove(Move* M)
    { // called by player objects when they want to move
        try {
            Ply(1 - s().SideToMove())->FollowMove(M);

            history.insert(std::next(current), GameState(s()));
            current++;

            s().MakeMove(M);
        } catch (std::exception& ex) {
            // If TypeOf ex Is KeyNotFoundException Then Throw
            std::cerr << "Exception in MakeMove\n"
                      << ex.what() << std::endl;
        }
    }

    void ApplySetup(GameState toSet)
    {
        history.insert(std::next(current), toSet);
        current++;
    }

    void CancelThinking()
    {
        for (int i = 0; i < 2; ++i) {
            Ply(i)->CancelThinking();
        }
    }

    bool PlayertypeChangingCmdAllowed()
    {
        // Return TypeOf Ply(s.SideToMove) Is HumanPlayer
        return true;
    }

    void CopyMoveList()
    {
        throw std::runtime_error("NotImplementedException");
    }
};

class GameState {
public:
    // The board (-1: empty, 0: white piece, 1: black piece)
    std::vector<int> T = std::vector<int>(24, -1);
    int phase = 1;
    // How many stones the players have set
    std::vector<int> SetStoneCount = std::vector<int>(2, 0);
    std::vector<int> StoneCount = std::vector<int>(2, 0);
    bool KLE = false; // Is there a puck removal coming?
    int SideToMove = 0;
    int MoveCount = 0;
    bool over = false;
    int winner = 0; // (-1, if a draw)
    bool block = false;
    int LastIrrev = 0;

    GameState() { } // start of game

    GameState(const GameState& s)
    { // copy constructor
        T = s.T;
        phase = s.phase;
        SetStoneCount = s.SetStoneCount;
        StoneCount = s.StoneCount;
        KLE = s.KLE;
        SideToMove = s.SideToMove;
        MoveCount = s.MoveCount;
        over = s.over;
        winner = s.winner;
        block = s.block;
        LastIrrev = s.LastIrrev;
    }

    int FutureStoneCount(int p)
    {
        return StoneCount[p] + MaxKSZ - SetStoneCount[p];
    }

    // Sets the state for Setup Mode: the placed stones are unchanged, but we switch to phase 2.
    void InitSetup()
    {
        MoveCount = 10; // Nearly all the same, just don't be too small, see other comments
        over = false;
        // Winner can be undefined, as over = False
        block = false;
        LastIrrev = 0;
    }

    void makeMove(Move* M)
    {
        if (M == nullptr) {
            throw std::invalid_argument("M is null");
        }

        checkInvariants();
        checkValidMove(M);

        moveCount++;

        SetKorong* sk = dynamic_cast<SetKorong*>(M);
        MoveKorong* mk = dynamic_cast<MoveKorong*>(M);
        LeveszKorong* lk = dynamic_cast<LeveszKorong*>(M);

        if (sk != nullptr) {
            T[sk->hov] = sideToMove;
            setStoneCount[sideToMove]++;
            stoneCount[sideToMove]++;
            lastIrrev = 0;
        } else if (mk != nullptr) {
            T[mk->hon] = -1;
            T[mk->hov] = sideToMove;
            lastIrrev++;
            if (lastIrrev >= lastIrrevLimit) {
                over = true;
                winner = -1; // draw
            }
        } else if (lk != nullptr) {
            T[lk->hon] = -1;
            stoneCount[1 - sideToMove]--;
            KLE = false;
            if (stoneCount[1 - sideToMove] + maxKSZ - setStoneCount[1 - sideToMove] < 3) {
                over = true;
                winner = sideToMove;
            }
            lastIrrev = 0;
        }

        if ((sk != nullptr || mk != nullptr) && malome(M->hov, this) > -1 && stoneCount[1 - sideToMove] > 0) {
            KLE = true;
        } else {
            sideToMove = 1 - sideToMove;
            if (setStoneCount[0] == maxKSZ && setStoneCount[1] == maxKSZ && phase == 1)
                phase = 2;
            if (!youCanMove(this)) {
                over = true;
                block = true;
                winner = 1 - sideToMove;
                if (wrappers::constants::FBD && stoneCount[0] == 12 && stoneCount[1] == 12) {
                    winner = -1;
                }
            }
        }

        checkInvariants();
    }

    void checkValidMove(BaseMove* M)
    {
        // Hard to ensure that the 'over and winner = -1' case never occurs. For example, the WithTaking case of PerfectPlayer.MakeMoveInState is tricky, because the previous MakeMove may have already made it a draw.
        assert(!over || winner == -1);

        SetKorong* sk = dynamic_cast<SetKorong*>(M);
        MoveKorong* mk = dynamic_cast<MoveKorong*>(M);
        LeveszKorong* lk = dynamic_cast<LeveszKorong*>(M);

        if (sk != nullptr) {
            assert(phase == 1);
            assert(T[sk->hov] == -1);
        }
        if (mk != nullptr) {
            assert(T[mk->hon] == sideToMove);
            assert(T[mk->hov] == -1);
        }
        if (lk != nullptr) {
            assert(KLE);
            assert(T[lk->hon] == 1 - sideToMove);
        }
    }

    void checkInvariants()
    {
        assert(setStoneCount[0] >= 0);
        assert(setStoneCount[0] <= rules::maxKSZ);
        assert(setStoneCount[1] >= 0);
        assert(setStoneCount[1] <= rules::maxKSZ);
        assert(phase == 1 || (phase == 2 && setStoneCount[0] == maxKSZ && setStoneCount[1] == maxKSZ));
    }

    // Called when applying a free setup. It sets over and checks whether the position is valid. Returns "" if valid, reason str otherwise.
    // Also called when pasting a position.
    std::string setOverAndCheckValidSetup()
    {
        assert(!over && !block);

        // Validity checks:
        // Note: this should be before setting over, because we will deny applying the setup if the state is not valid, and we want to maintain the 'Not over and Not block' invariants.

        int toBePlaced0 = rules::maxKSZ - setStoneCount[0];
        if (stoneCount[0] + toBePlaced0 > rules::maxKSZ) {
            return "Too many white stones (on the board + to be placed). Please remove some white stones from the board and/or decrease the number of white stones to be placed.";
        }
        int toBePlaced1 = rules::maxKSZ - setStoneCount[1];
        if (stoneCount[1] + toBePlaced1 > rules::maxKSZ) {
            return "Too many black stones (on the board + to be placed). Please remove some black stones from the board and/or decrease the number of black stones to be placed.";
        }

        assert(!(phase == 1 && toBePlaced0 == 0 && toBePlaced1 == 0));
        assert(!(phase == 2 && (toBePlaced0 > 0 || toBePlaced1 > 0)));

        if (wrappers::constants::variant != wrappers::constants::variants::lask && !wrappers::constants::extended) {
            if (phase == 1) {
                if (toBePlaced0 != toBePlaced1 - ((sideToMove == 0 ^ KLE) ? 0 : 1)) {
                    return "If Black is to move in the placement phase, then the number of black stones to be placed should be one more than the number of white stones to placed. If White is to move in the placement phase, then the number of white and black stones to be placed should be equal. (Except in a stone taking position, where these conditions are reversed.)\n\nNote: The Lasker variant (and the extended solutions) doesn't have these constraints.\n\nNote: You can switch the side to move by the \"Switch STM\" button in position setup mode.";
                }
            } else {
                assert(phase == 2);
                assert(toBePlaced0 == 0 && toBePlaced1 == 0);
            }
        }

        if (KLE && stoneCount[1 - sideToMove] == 0) {
            return "A position where the opponent doesn't have any stones cannot be a stone taking position.";
        }

        // Set over if needed:
        bool whiteLose = false, blackLose = false;
        if (stoneCount[0] + maxKSZ - setStoneCount[0] < 3) {
            whiteLose = true;
        }
        if (stoneCount[1] + maxKSZ - setStoneCount[1] < 3) {
            blackLose = true;
        }
        if (whiteLose || blackLose) {
            over = true;
            if (whiteLose && blackLose) {
                winner = -1; // draw
            } else {
                if (whiteLose) {
                    winner = 1;
                } else {
                    assert(blackLose);
                    winner = 0;
                }
            }
        }
        if (!KLE && !youCanMove(this)) { // youCanMove doesn't handle the KLE case. However, we should always have a move in KLE, see the validity check above.
            over = true;
            block = true;
            winner = 1 - sideToMove;
            if (wrappers::constants::FBD && stoneCount[0] == 12 && stoneCount[1] == 12) {
                winner = -1;
            }
        }

        // Even though LastIrrev is always 0 while in free setup mode, it can be non-0 when pasting
        if (lastIrrev >= rules::lastIrrevLimit) {
            over = true;
            winner = -1;
        }

        return "";
    }

    // to paste from clipboard
    GameState(const std::string& s)
    {
        std::vector<std::string> ss;
        std::string temp;
        std::stringstream strStream(s);

        // split by commas
        while (std::getline(strStream, temp, ',')) {
            ss.push_back(temp);
        }

        try {
            if (ss[33] == "malom" || ss[34] == "malom" || ss[35] == "malom" || ss[37] == "malom2") { // you need to be able to interpret older formats as well
                for (int i = 0; i < 24; i++) {
                    T[i] = std::stoi(ss[i]);
                }
                SideToMove = std::stoi(ss[24]);
                phase = std::stoi(ss[27]);
                SetStoneCount[0] = std::stoi(ss[28]);
                SetStoneCount[1] = std::stoi(ss[29]);
                StoneCount[0] = std::stoi(ss[30]);
                StoneCount[1] = std::stoi(ss[31]);
                KLE = (ss[32] == "True" || ss[32] == "true");
                MoveCount = (ss[33] != "malom") ? std::stoi(ss[33]) : 10;
                LastIrrev = ((ss[33] != "malom") && (ss[34] != "malom")) ? std::stoi(ss[34]) : 0;

                // ensure correct count of stones
                int count0 = std::count(T.begin(), T.end(), 0);
                int count1 = std::count(T.begin(), T.end(), 1);
                if (StoneCount[0] != count0 || StoneCount[1] != count1) {
                    throw InvalidGameStateException("Number of stones is incorrect.");
                }
            } else {
                throw std::invalid_argument("Invalid Format");
            }
        } catch (InvalidGameStateException& ex) {
            throw ex;
        } catch (std::exception& ex) {
            throw std::invalid_argument("Invalid Format");
        }
    }

    // for clipboard
    std::string toString()
    {
        std::stringstream s;
        for (int i = 0; i < 24; i++) {
            s << T[i] << ",";
        }
        s << SideToMove << "," << 0 << "," << 0 << "," << phase << "," << SetStoneCount[0]
          << "," << SetStoneCount[1] << "," << StoneCount[0] << "," << StoneCount[1]
          << "," << (KLE ? "True" : "False") << "," << MoveCount << "," << LastIrrev;
        return s.str();
    }
};

class InvalidGameStateException : public std::exception {
public:
    std::string mymsg;
    InvalidGameStateException(const std::string& msg)
        : mymsg(msg)
    {
    }

    virtual const char* what() const noexcept override
    {
        return mymsg.c_str();
    }
};
