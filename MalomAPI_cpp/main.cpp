#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <assert.h>

using namespace std;

// Assuming other classes and constants are defined elsewhere
class GameState {
public:
    std::vector<int> board { std::vector<int>(24, -1) };
    int phase = 1;
    std::vector<int> setStoneCount { std::vector<int>(2, 0) };
    std::vector<int> stoneCount { std::vector<int>(2, 0) };
    bool isPendingRemoval = false;
    int sideToMove = 0;
    int moveCount = 0;
    bool gameOver = false;
    int winner = 0;
    bool block = false;
    int lastIrrev = 0;

    int FutureStoneCount(int p)
    {
        return stoneCount[p] + maxStoneSize - setStoneCount[p];
    }

    // Copy constructor
    GameState(const GameState& s)
    {
        board = s.board;
        phase = s.phase;
        setStoneCount = s.setStoneCount;
        stoneCount = s.stoneCount;
        isPendingRemoval = s.isPendingRemoval;
        sideToMove = s.sideToMove;
        moveCount = s.moveCount;
        gameOver = s.gameOver;
        winner = s.winner;
        block = s.block;
        lastIrrev = s.lastIrrev;
    }

    // Sets the state for Setup Mode: the placed stones are unchanged, but we switch to phase 2.
    void InitSetup()
    {
        // T remains unchanged
        // phase remains unchanged
        // SetStoneCount remains unchanged
        // StoneCount remains unchanged, since T is unchanged
        // KLE remains unchanged
        // SideToMove remains unchanged
        MoveCount = 10; // almost doesn't matter, just shouldn't be too small, see comment elsewhere
        over = false;
        // Winner can be undefined, as over = False
        block = false;
        LastIrrev = 0;
    }

    void MakeMove(Object M)
    {
        if (typeid(M) != typeid(Move))
            throw std::invalid_argument("Invalid argument");

        CheckInvariants();
        CheckValidMove(M);

        MoveCount += 1;
        if (typeid(M) == typeid(SetKorong)) {
            T[M.hov] = SideToMove;
            SetStoneCount[SideToMove] += 1;
            StoneCount[SideToMove] += 1;
            LastIrrev = 0;
        } else if (typeid(M) == typeid(MoveKorong)) {
            T[M.hon] = -1;
            T[M.hov] = SideToMove;
            LastIrrev += 1;
            if (LastIrrev >= LastIrrevLimit) {
                over = true;
                winner = -1; // draw
            }
        } else if (typeid(M) == typeid(LeveszKorong)) {
            T[M.hon] = -1;
            StoneCount[1 - SideToMove] -= 1;
            KLE = false;
            if (StoneCount[1 - SideToMove] + MaxKSZ - SetStoneCount[1 - SideToMove] < 3) { // TODO: refactor to call to FutureStoneCount
                over = true;
                winner = SideToMove;
            }
            LastIrrev = 0;
        }
        if ((typeid(M) == typeid(SetKorong) || typeid(M) == typeid(MoveKorong)) && Malome(M.hov, this) > -1 && StoneCount[1 - SideToMove] > 0) { // if he made a monkey, your move is your opponent's puck
            KLE = true;
        } else {
            SideToMove = 1 - SideToMove;
            if (SetStoneCount[0] == MaxKSZ && SetStoneCount[1] == MaxKSZ && phase == 1)
                phase = 2; // switching to disc movement
            if (!YouCanMove(this)) {
                over = true;
                block = true;
                winner = 1 - SideToMove;
                if (Wrappers.Constants.FBD && StoneCount[0] == 12 && StoneCount[1] == 12) {
                    winner = -1;
                }
            }
        }

        CheckInvariants();
    }

    // Check that the move is valid
    void CheckValidMove(Move M)
    {
        assert(!over || winner == -1); // It's difficult to ensure that 'over and winner = -1' never occurs. E.g. the WithTaking case in PerfectPlayer.MakeMoveInState can be problematic, because the previous MakeMove might have resulted in a draw
        if (typeid(M) == typeid(SetKorong)) {
            assert(phase == 1);
            SetKorong setKorong = dynamic_cast<SetKorong&>(M);
            assert(T[setKorong.hov] == -1);
        }
        if (typeid(M) == typeid(MoveKorong)) {
            MoveKorong slide = dynamic_cast<MoveKorong&>(M);
            assert(T[slide.hon] == SideToMove);
            assert(T[slide.hov] == -1);
        }
        if (typeid(M) == typeid(LeveszKorong)) {
            assert(KLE);
            LeveszKorong take = dynamic_cast<LeveszKorong&>(M);
            assert(T[take.hon] == 1 - SideToMove);
        }
    }

    // Checks that the invariants hold
    void CheckInvariants()
    {
        assert(SetStoneCount[0] >= 0);
        assert(SetStoneCount[0] <= Rules::MaxKSZ);
        assert(SetStoneCount[1] >= 0);
        assert(SetStoneCount[1] <= Rules::MaxKSZ);
        assert(phase == 1 || (phase == 2 && SetStoneCount[0] == MaxKSZ && SetStoneCount[1] == MaxKSZ));
    }

    GameState(const std::string& s)
    {
        std::istringstream ss(s);
        std::string token;
        std::vector<std::string> tokens;
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }
        try {
            if (tokens[33] == "malom" || tokens[34] == "malom" || tokens[35] == "malom" || tokens[37] == "malom2") {
                for (int i = 0; i < 24; i++) {
                    T[i] = stoi(tokens[i]);
                }
                SideToMove = stoi(tokens[24]);
                phase = stoi(tokens[27]);
                SetStoneCount[0] = stoi(tokens[28]);
                SetStoneCount[1] = stoi(tokens[29]);
                StoneCount[0] = stoi(tokens[30]);
                StoneCount[1] = stoi(tokens[31]);
                KLE = stoi(tokens[32]);
                MoveCount = (tokens[33] != "malom") ? stoi(tokens[33]) : 10;
                LastIrrev = (tokens[33] != "malom" && tokens[34] != "malom") ? stoi(tokens[34]) : 0;
                if (StoneCount[0] != std::count(T.begin(), T.end(), 0) || StoneCount[1] != std::count(T.begin(), T.end(), 1)) {
                    throw InvalidGameStateException(" Number of stones is incorrect.");
                }
            } else {
                throw std::invalid_argument("Invalid format");
            }
        } catch (InvalidGameStateException& ex) {
            throw ex;
        } catch (...) {
            throw std::invalid_argument("Invalid format");
        }
    }

    std::string toString()
    {
        std::ostringstream s;
        for (int i = 0; i < 24; i++) {
            s << T[i] << ",";
        }
        s << SideToMove << "," << 0 << "," << 0 << "," << phase << "," << SetStoneCount[0] << "," << SetStoneCount[1] << "," << StoneCount[0] << "," << StoneCount[1] << "," << KLE << "," << MoveCount << "," << LastIrrev;
        return s.str();
    }
};

int main()
{
    cout << "Hello World!" << endl;

#ifdef WIN32
    system("pause");
#endif
    return 0;
}