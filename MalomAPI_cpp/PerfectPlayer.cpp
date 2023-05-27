#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cassert>
#include <functional>
#include <algorithm>
#include <exception>
#include <random>

#include "Constants.h"
#include "Rules.h"
#include "Sector.h"
#include "Wrappers.h"

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
                                id newId(w, b, wf, bf);
                                std::ifstream f(fname.c_str());
                                if (f.good()) {
                                    sectors[newId] = Sector(newId);
                                }
                            }
                        }
                    }
                }

                created = true;
            }

            return sectors;
        } catch (const std::exception& ex) {
            std::cerr << "An error happened in getsectors" << std::endl
                      << ex.what() << std::endl;
            std::exit(1);
            return {}; // Return an empty map
        }
    }

    static bool HasDatabase()
    {
        return getsectors().size() > 0;
    }
};

// Initialize static members
std::map<id, Sector> Sectors::sectors;
bool Sectors::created = false;

class PerfectPlayer : public Player {
public:
    std::map<id, Sector> secs;
    static const bool UseWRGM = false;
    Engine* Eng = nullptr;

    PerfectPlayer()
    {
        assert(Sectors::HasDatabase());
        secs = Sectors::getsectors();

        if (UseWRGM) {
            assert(Rules::AlphaBetaAvailable());
            Eng = new Engine(nullptr, nullptr, nullptr, true);
            Eng->InitEngine();
        }
    }

    void Enter(Game _g) override
    {
        Player::Enter(_g);
    }

    void Quit() override
    {
        Player::Quit();
    }

    Sector* GetSec(GameState s)
    {
        try {
            if (s.KLE)
                return nullptr;

            id newId(s.StoneCount[0], s.StoneCount[1], Rules.MaxKSZ - s.SetStoneCount[0], Rules.MaxKSZ - s.SetStoneCount[1]);

            if (s.SideToMove == 1) {
                newId.negate();
            }

            // Assume that "secs" is a std::unordered_map or similar that can throw std::out_of_range
            return &secs.at(newId);
        } catch (const std::exception& ex) {
            if (typeid(ex) == typeid(std::out_of_range)) {
                throw;
            }
            std::cerr << "An error happened in GetSec" << std::endl
                      << ex.what() << std::endl;
            exit(1);
            return nullptr;
        }
    }

    GameState NegateState(GameState s)
    {
        GameState r = s; // this assumes that GameState has a copy constructor
        for (int i = 0; i < 24; ++i) {
            if (r.T[i] == 0) {
                r.T[i] = 1;
            } else if (r.T[i] == 1) {
                r.T[i] = 0;
            }
        }
        r.StoneCount[0] = s.StoneCount[1];
        r.StoneCount[1] = s.StoneCount[0];
        r.SetStoneCount[0] = s.SetStoneCount[1];
        r.SetStoneCount[1] = s.SetStoneCount[0];
        return r;
    }

    void OppToMove(GameState s) override
    {
        Player::OppToMove(s);
        try {
            Main->LblPerfEval.Visible = Main->Settings.ShowEv;
            if (!s.KLE) {
                Main->LblPerfEvalSetText("Eval: " + ToHumanReadableEval(Eval(s)) + ", NGM: " + std::to_string(NumGoodMoves(s)));
            } else {
                Main->LblPerfEvalSetText("Eval: " + ToHumanReadableEval(MoveValue(s, GoodMoves(s).front())) + ", NGM: " + std::to_string(NumGoodMoves(s)));
            }
        } catch (std::out_of_range& ex) {
            Main->LblPerfEvalSetText("NO DATABASE FILE; NOT PLAYING PERFECTLY", true);
        } catch (std::exception& ex) {
            std::cerr << "An error happened in OppToMove" << std::endl
                      << ex.what() << std::endl;
            std::exit(1);
        }
    }

    std::string ToHumanReadableEval(gui_eval_elem2 e)
    {
        try {
            return e.toString();
        } catch (const std::exception& ex) {
            std::cerr << "An error happened in ToHumanReadableEval" << std::endl
                      << ex.what() << std::endl;
            exit(1);
            return "";
        }
    }

    enum class MoveType {
        SetMove,
        SlideMove // should be renamed to SlideOrJumpMove
    };

    struct Move {
        int hon, hov;
        MoveType moveType;
        bool withTaking, onlyTaking;
        int takeHon;

        int toBitBoard() const
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

    int futureKorongCount(const GameState& s)
    {
        // TODO: refactor to call to FutureStoneCount
        return s.stoneCount(s.sideToMove()) + Rules::MaxKSZ - s.setStoneCount(s.sideToMove());
    }

    bool makesMill(const GameState& s, int hon, int hov)
    {
        GameState s2(s);
        if (hon != -1)
            s2.T(hon) = -1;
        s2.T(hov) = s.sideToMove();
        return -1 != Rules::Malome(hov, s2);
    }

    bool isMill(const GameState& s, int m)
    {
        return -1 != Rules::Malome(m, s);
    }

    std::vector<Move> setMoves(const GameState& s)
    {
        std::vector<Move> r;
        for (int i = 0; i < 24; ++i) {
            if (s.T(i) == -1) {
                Move move;
                move.hov = i;
                move.withTaking = makesMill(s, -1, i);
                move.moveType = MoveType::SetMove;
                r.push_back(move);
            }
        }
        return r;
    }

    std::vector<Move> slideMoves(const GameState& s)
    {
        std::vector<Move> r;
        for (int i = 0; i < 24; ++i) {
            for (int j = 0; j < 24; ++j) {
                if (s.T(i) == s.sideToMove() && s.T(j) == -1 && (futureKorongCount(s) == 3 || Rules::BoardGraph(i, j))) {
                    Move move;
                    move.hon = i;
                    move.hov = j;
                    move.withTaking = makesMill(s, i, j);
                    move.moveType = MoveType::SlideMove;
                    r.push_back(move);
                }
            }
        }
        return r;
    }

    // "m" contains a WithTaking move, but without TakeHon filled. This function creates a list with copies of "m", each one complemented with a possible removal.
    std::vector<Move> withTakingMoves(const GameState& s, Move m)
    {
        std::vector<Move> r;

        bool everythingInMill = true;
        for (int i = 0; i < 24; ++i) {
            if (s.T(i) == 1 - s.sideToMove() && !isMill(s, i))
                everythingInMill = false;
        }

        for (int i = 0; i < 24; ++i) {
            if (s.T(i) == 1 - s.sideToMove() && (!isMill(s, i) || everythingInMill)) {
                Move m2 = m;
                m2.takeHon = i;
                r.push_back(m2);
            }
        }

        return r;
    }

    // This is similar to withTakingMoves, but focuses only on taking moves. Some code duplication present.
    std::vector<Move> onlyTakingMoves(const GameState& s)
    {
        std::vector<Move> r;

        bool everythingInMill = true;
        for (int i = 0; i < 24; ++i) {
            if (s.T(i) == 1 - s.sideToMove() && !isMill(s, i))
                everythingInMill = false;
        }

        for (int i = 0; i < 24; ++i) {
            if (s.T(i) == 1 - s.sideToMove() && (!isMill(s, i) || everythingInMill)) {
                Move move;
                move.onlyTaking = true;
                move.takeHon = i;
                r.push_back(move);
            }
        }

        return r;
    }

    std::vector<Move> getMoveList(const GameState& s)
    {
        std::vector<Move> ms0, ms;
        if (!s.KLE()) {
            if (Wrappers::Constants::Variant == Wrappers::Constants::Variants::std || Wrappers::Constants::Variant == Wrappers::Constants::Variants::mora) {
                if (s.setStoneCount(s.sideToMove()) < Rules::MaxKSZ) {
                    ms0 = setMoves(s);
                } else {
                    ms0 = slideMoves(s);
                }
            } else { // Lasker
                ms0 = slideMoves(s);
                if (s.setStoneCount(s.sideToMove()) < Rules::MaxKSZ) {
                    std::vector<Move> setMovesList = setMoves(s);
                    ms0.insert(ms0.end(), setMovesList.begin(), setMovesList.end());
                }
            }

            for (int i = 0; i < ms0.size(); ++i) {
                if (!ms0[i].withTaking) {
                    ms.push_back(ms0[i]);
                } else {
                    std::vector<Move> withTakingMovesList = withTakingMoves(s, ms0[i]);
                    ms.insert(ms.end(), withTakingMovesList.begin(), withTakingMovesList.end());
                }
            }
        } else { // KLE
            ms = onlyTakingMoves(s);
        }

        return ms;
    }

    GameState makeMoveInState(const GameState& s, const Move& m)
    {
        GameState s2 = s;
        if (!m.onlyTaking) {
            if (m.moveType == MoveType::SetMove) {
                s2.makeMove(SetKorong(m.hov));
            } else {
                s2.makeMove(MoveKorong(m.hon, m.hov));
            }
            if (m.withTaking)
                s2.makeMove(LeveszKorong(m.takeHon));
        } else {
            s2.makeMove(LeveszKorong(m.takeHon));
        }
        return s2;
    }

    gui_eval_elem2 moveValue(const GameState& s, const Move& m)
    {
        try {
            return eval(makeMoveInState(s, m)).undo_negate(getSec(s));
        } catch (std::exception& e) {
            std::cerr << "Exception in MoveValue\n"
                      << e.what() << std::endl;
            std::exit(1);
        }
    }

    // Returns all elements of l, that are maximal by f
    template <typename T, typename K>
    std::vector<T> allMaxBy(std::function<K(T)> f, const std::vector<T>& l, K minValue)
    {
        std::vector<T> r;
        K ma = minValue;
        for (const auto& m : l) {
            K e = f(m);
            if (e > ma) {
                ma = e;
                r.clear();
                r.push_back(m);
            } else {
                if (e == ma) {
                    r.push_back(m);
                }
            }
        }
        return r;
    }

    std::vector<Move> goodMoves(const GameState& s)
    {
        return allMaxBy([this](const Move& m) { return moveValue(s, m); }, getMoveList(s), gui_eval_elem2::min_value(getSec(s)));
    }

    int NGMAfterMove(const GameState& s, const Move& m)
    {
        return numGoodMoves(makeMoveInState(s, m));
    }

    std::vector<Move> minNGMMoves(const GameState& s)
    {
        return allMaxBy([this](const Move& m) { return -NGMAfterMove(s, m); }, goodMoves(s), std::numeric_limits<int>::min());
    }

    int WRGMAfterMove(const GameState& s, const Move& m)
    {
        return WRGM(makeMoveInState(s, m));
    }

    std::vector<Move> minWRGMMoves(const GameState& s)
    {
        return allMaxBy([this](const Move& m) { return -WRGMAfterMove(s, m); }, goodMoves(s), std::numeric_limits<int>::min());
    }

    // Initialize a random number generator
    std::default_random_engine generator(std::random_device {}());

    template <typename T>
    T chooseRandom(const std::vector<T>& l)
    {
        std::uniform_int_distribution<int> distribution(0, l.size() - 1);
        return l[distribution(generator)];
    }

    void sendMoveToGUI(const Move& m)
    {
        if (!m.onlyTaking) {
            if (m.moveType == MoveType::SetMove) {
                G.makeMove(SetKorong(m.hov));
            } else {
                G.makeMove(MoveKorong(m.hon, m.hov));
            }
        } else {
            G.makeMove(LeveszKorong(m.takeHon));
        }
    }

    // Treatment of the disc herniation:
    // -Eval cannot handle KLE allas, since they are not stored from the solver's point of view.
    // -So, in the move structure here, the mill nozzle is part of the disc juice leaf.
    // - However, from the point of view of the GUI, this is a different event.
    // -Vegulis has become that when we get KLE support from the GUI, we look into the database again.
    // (This is how we can handle it if, for example, KLE allas is pasted (as well as it fits better with the regi engine))
    void ToMove(GameState s) override
    {
        try {
            Move mh = ChooseRandom(GoodMoves(s));
            SendMoveToGUI(mh);
        } catch (std::out_of_range& ex) {
            Main->LblPerfEvalSetText("NO DATABASE FILE; NOT PLAYING PERFECTLY", true);
            SendMoveToGUI(ChooseRandom(GetMoveList(s)));
        } catch (std::exception& ex) {
            std::cerr << "Exception in ToMove" << std::endl
                      << ex.what() << std::endl;
            std::exit(1);
        }
    }

    int numGoodMoves(const GameState& s)
    {
        if (futureKorongCount(s) < 3)
            return 0;
        auto ma = gui_eval_elem2::min_value(getSec(s));
        Move mh;
        int c = 0;
        for (const auto& m : getMoveList(s)) {
            auto e = moveValue(s, m);
            if (e > ma) {
                ma = e;
                mh = m;
                c = 1;
            } else if (e == ma) {
                c += 1;
            }
        }
        return c;
    }

    double WRGM(const GameState& s)
    {
        if (!useWRGM)
            throw std::runtime_error("Turn on useWRGM");

        if (futureKorongCount(s) < 3)
            return 0;
        auto ma = gui_eval_elem2::min_value(getSec(s));
        Move mh;
        double numer = 0.0, denom = 0.0;
        auto moveList = getMoveList(s);
        for (const auto& m : moveList) {
            auto e = moveValue(s, m);

            double h = eng.think(std::make_tuple(s, 0.1)).ev;
            h /= 1000000;
            if (h > 10)
                h = 10;
            if (h < 0.1)
                h = 0.1;
            const double a = 20.0;
            double weight = std::exp(a * std::log(h));

            if (e > ma) {
                ma = e;
                mh = m;
                numer = weight;
            } else if (e == ma) {
                numer += weight;
            }
            denom += weight;
        }
        if (moveList.empty()) {
            return 1;
        } else {
            return numer / denom;
        }
    }

    int cp;
    struct MoveValuePair {
        Move m;
        double val;
    };
    const double WRGMInf = 2.0;
    MoveValuePair recWRGMInner(GameState& s, int d, double alpha, double beta)
    {
        if (d == 0) {
            assert(s.sideToMove == 1 - cp);
            return { Move(), WRGM(s) };
        }
        MoveValuePair ma = { Move(), -WRGMInf };
        auto ml = goodMoves(s);
        auto w = s.sideToMove == 1 - cp ? WRGM(s) : 1;
        for (const auto& m : ml) {
            auto s2 = makeMoveInState(s, m);
            auto a = recWRGMInner(s2, d - 1, -beta / w, -std::max(alpha, ma.val) / w);
            a.val *= -1;
            if (a.val > ma.val)
                ma = { m, a.val };
            if (a.val > beta) {
                goto cutoff;
            }
        }
    cutoff:
        ma.val *= w;
        return ma;
    }

    MoveValuePair recWRGM(GameState& s)
    {
        throw std::runtime_error("RecWRGM is not yet rewritten to distinguish draws");
    }

    std::mutex evalLock;
    gui_eval_elem2 eval(const GameState& s)
    {
        std::lock_guard<std::mutex> lock(evalLock);
        assert(!s.KLE);
        auto id = Id(s.stoneCount[0], s.stoneCount[1], rules::maxKSZ - s.setStoneCount[0], rules::maxKSZ - s.setStoneCount[1]);

        if (futureKorongCount(s) < 3)
            return gui_eval_elem2::virt_loss_val;

        int64_t a = 0;
        for (int i = 0; i < 24; ++i) {
            if (s.T[i] == 0) {
                a = a | (int64_t(1) << i);
            } else if (s.T[i] == 1) {
                a = a | (int64_t(1) << (i + 24));
            }
        }

        if (s.sideToMove == 1) {
            a = boardNegate(a);
            id.negate();
        }

        auto sec = secs[id];

        return sec.hash(a).second;
    }

    int64_t boardNegate(int64_t a)
    {
        const int64_t mask24 = (1 << 24) - 1;
        return ((a & mask24) << 24) | ((a & (mask24 << 24)) >> 24);
    }    
};