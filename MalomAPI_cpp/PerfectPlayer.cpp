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

class id {
    // Define your class 'id' here
};

class Sector {
public:
    Sector(id _id)
    {
        // Define your constructor here
    }
};

class Sectors {
public:
    static std::map<id, Sector> sectors;
    static bool created;

    static std::map<id, Sector> getsectors()
    {
        try {
            if (!created) {
                Wrappers::Init::init_sym_lookuptables();
                Wrappers::Init::init_sec_vals();

                for (int w = 0; w <= Rules::MaxKSZ; ++w) {
                    for (int b = 0; b <= Rules::MaxKSZ; ++b) {
                        for (int wf = 0; wf <= Rules::MaxKSZ; ++wf) {
                            for (int bf = 0; bf <= Rules::MaxKSZ; ++bf) {
                                std::string fname = Rules::VariantName + "_" + std::to_string(w) + "_" + std::to_string(b) + "_" + std::to_string(wf) + "_" + std::to_string(bf) + ".sec" + Constants::Fname_suffix;
                                // std::cout << "Looking for database file " << fname << std::endl;
                                id _id(w, b, wf, bf);
                                std::ifstream file(fname);
                                if (file.good()) {
                                    sectors[_id] = Sector(_id);
                                }
                            }
                        }
                    }
                }
                created = true;
            }
            return sectors;
        } catch (std::exception& ex) {
            if (dynamic_cast<std::out_of_range*>(&ex)) {
                throw;
            }
            std::cerr << "An error happened in getsectors\n"
                      << ex.what() << std::endl;
            exit(1);
        }
    }

    static bool HasDatabase()
    {
        return getsectors().size() > 0;
    }
};

// Initialize static member variables
std::map<id, Sector> Sectors::sectors;
bool Sectors::created = false;

class PerfectPlayer : public Player {
public:
    std::map<id, Sector> secs;

    PerfectPlayer()
    {
        assert(Sectors::HasDatabase());
        secs = Sectors::getsectors();
    }

    void Enter(Game _g) override
    {
        Player::Enter(_g);
    }

    void Quit() override
    {
        Player::Quit();
    }

    Sector* PerfectPlayer::GetSec(GameState s)
    {
        try {
            if (s.KLE)
                return nullptr;

            id id_val(s.StoneCount[0], s.StoneCount[1], Rules::MaxKSZ - s.SetStoneCount[0], Rules::MaxKSZ - s.SetStoneCount[1]);

            if (s.SideToMove == 1) {
                id_val.negate();
            }

            return &secs[id_val];
        } catch (std::exception& ex) {
            if (typeid(ex) == typeid(std::out_of_range))
                throw;
            std::cerr << "An error happened in GetSec\n"
                      << ex.what() << std::endl;
            std::exit(1);
        }
        return nullptr;
    }

    std::string PerfectPlayer::ToHumanReadableEval(gui_eval_elem2 e)
    {
        try {
            return e.ToString();
        } catch (std::exception& ex) {
            std::cerr << "An error happened in ToHumanReadableEval\n"
                      << ex.what() << std::endl;
            std::exit(1);
        }
        return "";
    }

    enum class MoveType {
        SetMove,
        SlideMove // should be renamed to SlideOrJumpMove
    };

    struct Move {
        int hon, hov;
        MoveType movetype;
        bool withtaking, onlytaking; // withtaking includes the steps in mill closure, onlytaking only includes removal
        int takehon;

        int toBitBoard()
        {
            if (onlytaking) {
                return 1 << takehon;
            }
            int ret = 1 << hov;
            if (movetype == MoveType::SlideMove) {
                ret += 1 << hon;
            }
            if (withtaking) {
                ret += 1 << takehon;
            }
            return ret;
        }
    };

    int futureKorongCount(GameState& s)
    {
        return s.stoneCount(s.sideToMove) + Rules.maxKSZ - s.setStoneCount(s.sideToMove); // TODO: refactor to call to futureStoneCount
    }

    bool makesMill(GameState& s, int hon, int hov)
    {
        GameState s2 = s;
        if (hon != -1)
            s2.T(hon) = -1;
        s2.T(hov) = s.sideToMove;
        return -1 != Rules.malome(hov, s2);
    }

    bool isMill(GameState& s, int m)
    {
        return -1 != Rules.malome(m, s);
    }

    std::vector<Move> setMoves(GameState& s)
    {
        std::vector<Move> r;
        for (int i = 0; i < 24; ++i) {
            if (s.T(i) == -1) {
                r.push_back(Move { i, false, makesMill(s, -1, i), MoveType::SetMove });
            }
        }
        return r;
    }

    std::vector<Move> slideMoves(GameState& s)
    {
        std::vector<Move> r;
        for (int i = 0; i < 24; ++i) {
            for (int j = 0; j < 24; ++j) {
                if (s.T(i) == s.sideToMove && s.T(j) == -1 && (futureKorongCount(s) == 3 || Rules.boardGraph(i, j))) {
                    r.push_back(Move { i, j, makesMill(s, i, j), false, MoveType::SlideMove, 0 });
                }
            }
        }
        return r;
    }

    // m has a withtaking step, where takehon is not filled out. This function creates a list, the elements of which are copies of m supplemented with one possible removal each.
    std::vector<Move> withTakingMoves(GameState& s, Move& m)
    {
        std::vector<Move> r;
        bool everythingInMill = true;
        for (int i = 0; i < 24; ++i) {
            if (s.T(i) == 1 - s.sideToMove && !isMill(s, i)) {
                everythingInMill = false;
            }
        }

        for (int i = 0; i < 24; ++i) {
            if (s.T(i) == 1 - s.sideToMove && (!isMill(s, i) || everythingInMill)) {
                Move m2 = m;
                m2.takehon = i;
                r.push_back(m2);
            }
        }
        return r;
    }

    std::vector<Move> onlyTakingMoves(GameState& s)
    { // there's some copy-paste code here
        std::vector<Move> r;
        bool everythingInMill = true;
        for (int i = 0; i < 24; ++i) {
            if (s.T(i) == 1 - s.sideToMove && !isMill(s, i)) {
                everythingInMill = false;
            }
        }

        for (int i = 0; i < 24; ++i) {
            if (s.T(i) == 1 - s.sideToMove && (!isMill(s, i) || everythingInMill)) {
                r.push_back(Move { 0, 0, false, true, MoveType::SlideMove, i }); // Assuming default values for hon, hov, and movetype
            }
        }
        return r;
    }

    std::vector<Move> getMoveList(GameState& s)
    {
        std::vector<Move> ms0, ms;
        if (!s.KLE) {
            if (Wrappers.Constants.variant == Wrappers.Constants.Variants.std || Wrappers.Constants.variant == Wrappers.Constants.Variants.mora) {
                if (s.setStoneCount(s.sideToMove) < Rules.maxKSZ) {
                    ms0 = setMoves(s);
                } else {
                    ms0 = slideMoves(s);
                }
            } else { // Lasker
                ms0 = slideMoves(s);
                if (s.setStoneCount(s.sideToMove) < Rules.maxKS Z) {
                    std::vector<Move> setMovesResult = setMoves(s);
                    ms0.insert(ms0.end(), setMovesResult.begin(), setMovesResult.end());
                }
            }

            for (int i = 0; i < ms0.size(); ++i) {
                if (!ms0[i].withtaking) {
                    ms.push_back(ms0[i]);
                } else {
                    std::vector<Move> withTakingMovesResult = withTakingMoves(s, ms0[i]);
                    ms.insert(ms.end(), withTakingMovesResult.begin(), withTakingMovesResult.end());
                }
            }
        } else { // KLE
            ms = onlyTakingMoves(s);
        }
        return ms;
    }

    GameState makeMoveInState(GameState& s, Move& m)
    {
        GameState s2(s);
        if (!m.onlyTaking) {
            if (m.moveType == MoveType::SetMove) {
                s2.makeMove(SetKorong(m.hov)); // Assuming the definition of SetKorong class is available
            } else {
                s2.makeMove(MoveKorong(m.hon, m.hov)); // Assuming the definition of MoveKorong class is available
            }
            if (m.withTaking)
                s2.makeMove(LeveszKorong(m.takeHon)); // Assuming the definition of LeveszKorong class is available
        } else {
            s2.makeMove(LeveszKorong(m.takeHon));
        }
        return s2;
    }

    // Assuming gui_eval_elem2 and getSec functions are defined somewhere
    gui_eval_elem2 moveValue(GameState& s, Move& m)
    {
        try {
            return eval(makeMoveInState(s, m)).undo_negate(getSec(s));
        } catch (const std::exception& ex) {
            std::cerr << "Exception in MoveValue\n"
                      << ex.what() << std::endl;
            std::exit(1);
        }
    }

    template <typename T, typename K>
    std::vector<T> allMaxBy(std::function<K(T)> f, std::vector<T>& l, K minValue)
    {
        std::vector<T> r;
        K ma = minValue;
        for (auto& m : l) {
            K e = f(m);
            if (e > ma) {
                ma = e;
                r.clear();
                r.push_back(m);
            } else if (e == ma) {
                r.push_back(m);
            }
        }
        return r;
    }

    // Assuming the definition of gui_eval_elem2::min_value function
    std::vector<Move> goodMoves(GameState& s)
    {
        return allMaxBy([s](Move m) { return moveValue(s, m); }, getMoveList(s), gui_eval_elem2::min_value(getSec(s)));
    }

    int NGMAfterMove(GameState& s, Move& m)
    {
        return numGoodMoves(makeMoveInState(s, m)); // Assuming numGoodMoves function is defined
    }

    template <typename T>
    T chooseRandom(std::vector<T>& l)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, l.size() - 1);
        return l[dis(gen)];
    }

    void sendMoveToGUI(Move& m)
    {
        if (!m.onlyTaking) {
            if (m.moveType == MoveType::SetMove) {
                G.makeMove(SetKorong(m.hov)); // Assuming the definition of SetKorong class is available
            } else {
                G.makeMove(MoveKorong(m.hon, m.hov)); // Assuming the definition of MoveKorong class is available
            }
        } else {
            G.makeMove(LeveszKorong(m.takeHon)); // Assuming the definition of LeveszKorong class is available
        }
    }

    void toMove(GameState& s)
    {
        try {
            Move mh = chooseRandom(goodMoves(s));
            sendMoveToGUI(mh);
        } catch (const std::out_of_range&) {
            sendMoveToGUI(chooseRandom(getMoveList(s)));
        } catch (const std::exception& ex) {
            std::cerr << "Exception in ToMove\n"
                      << ex.what() << std::endl;
            std::exit(1);
        }
    }

    int numGoodMoves(GameState& s)
    {
        if (futureKorongCount(s) < 3)
            return 0; // Assuming futureKorongCount function is defined
        auto ma = gui_eval_elem2::min_value(getSec(s)); // Assuming getSec function is defined
        Move mh;
        int c = 0;
        for (auto& m : getMoveList(s)) {
            auto e = moveValue(s, m);
            if (e > ma) {
                ma = e;
                mh = m;
                c = 1;
            } else if (e == ma) {
                c++;
            }
        }
        return c;
    }

    int cp;

    struct MoveValuePair {
        Move m;
        double val;
    };

    const double WRGMInf = 2; // Is this good?

    std::mutex evalLock;

    gui_eval_elem2 eval(GameState& s)
    {
        try {
            std::lock_guard<std::mutex> lock(evalLock);
            assert(!s.KLE); // Assuming s has a boolean member KLE

            id Id(s.stoneCount[0], s.stoneCount[1], Rules::MaxKSZ - s.setStoneCount[0], Rules::MaxKSZ - s.setStoneCount[1]);

            if (futureKorongCount(s) < 3)
                return gui_eval_elem2::virt_loss_val;

            int64_t a = 0;
            for (int i = 0; i < 24; ++i) {
                if (s.T[i] == 0) {
                    a |= (1ll << i);
                } else if (s.T[i] == 1) {
                    a |= (1ll << (i + 24));
                }
            }

            if (s.sideToMove == 1) {
                a = boardNegate(a);
                id.negate();
            }

            sec sec = secs[id];

            return sec.hash(a).item2;
        } catch (const std::exception& ex) {
            if (typeid(ex) == typeid(std::out_of_range))
                throw;
            std::cerr << "Exception in Eval\n"
                      << ex.what() << std::endl;
            std::exit(1);
        }
    }

    int64_t boardNegate(int64_t a)
    {
        const int64_t mask24 = (1ll << 24) - 1;
        return ((a & mask24) << 24) | ((a & (mask24 << 24)) >> 24);
    }
};


