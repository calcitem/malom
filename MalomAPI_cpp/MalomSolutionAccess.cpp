#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class MalomSolutionAccess {
private:
    static std::shared_ptr<PerfectPlayer> pp;
    static std::exception_ptr lastError;

public:
    // I have left out the implementation details here as they depend on your specific needs and the definitions of PerfectPlayer, GameState, and other methods/classes
    static int GetBestMove(int whiteBitboard, int blackBitboard, int whiteStonesToPlace, int blackStonesToPlace, int playerToMove, bool onlyStoneTaking);
    static int GetBestMoveNoException(int whiteBitboard, int blackBitboard, int whiteStonesToPlace, int blackStonesToPlace, int playerToMove, bool onlyStoneTaking);
    static std::string GetLastError();
    static int GetBestMoveStr(std::string args);

private:
    static void InitializeIfNeeded();
    static void MustBeBetween(std::string paramName, int value, int min, int max);
    static void SetVariantStripped();
};

std::shared_ptr<PerfectPlayer> MalomSolutionAccess::pp = nullptr;
std::exception_ptr MalomSolutionAccess::lastError = nullptr;

int MalomSolutionAccess::GetBestMove(int whiteBitboard, int blackBitboard, int whiteStonesToPlace, int blackStonesToPlace, int playerToMove, bool onlyStoneTaking)
{
    InitializeIfNeeded();

    auto s = std::make_shared<GameState>();

    const int W = 0;
    const int B = 1;

    if ((whiteBitboard & blackBitboard) != 0) {
        throw std::invalid_argument("whiteBitboard and blackBitboard shouldn't have any overlap");
    }
    for (int i = 0; i <= 23; i++) {
        if ((whiteBitboard & (1 << i)) != 0) {
            s->T[i] = W;
            s->StoneCount[W] += 1;
        }
        if ((blackBitboard & (1 << i)) != 0) {
            s->T[i] = B;
            s->StoneCount[B] += 1;
        }
    }

    s->phase = (whiteStonesToPlace == 0 && blackStonesToPlace == 0) ? 2 : 1;
    MustBeBetween("whiteStonesToPlace", whiteStonesToPlace, 0, Rules::MaxKSZ);
    MustBeBetween("blackStonesToPlace", blackStonesToPlace, 0, Rules::MaxKSZ);
    s->SetStoneCount[W] = Rules::MaxKSZ - whiteStonesToPlace;
    s->SetStoneCount[B] = Rules::MaxKSZ - blackStonesToPlace;
    s->KLE = onlyStoneTaking;
    MustBeBetween("playerToMove", playerToMove, 0, 1);
    s->SideToMove = playerToMove;
    s->MoveCount = 10;

    if (s->FutureStoneCount[W] > Rules::MaxKSZ) {
        throw std::invalid_argument("Number of stones in whiteBitboard + whiteStonesToPlace > " + std::to_string(Rules::MaxKSZ));
    }
    if (s->FutureStoneCount[B] > Rules::MaxKSZ) {
        throw std::invalid_argument("Number of stones in blackBitboard + blackStonesToPlace > " + std::to_string(Rules::MaxKSZ));
    }

    std::string errorMsg = s->SetOverAndCheckValidSetup();
    if (errorMsg != "") {
        throw std::invalid_argument(errorMsg);
    }
    if (s->over) {
        throw std::invalid_argument("Game is already over.");
    }

    s->LastIrrev = 0;

    try {
        return pp->ChooseRandom(pp->GoodMoves(s))->ToBitBoard();
    } catch (std::out_of_range&) {
        throw std::runtime_error("We don't have a database entry for this position. This can happen either if the database is corrupted (missing files), or sometimes when the position is not reachable from the starting position.");
    }
}

int MalomSolutionAccess::GetBestMoveNoException(int whiteBitboard, int blackBitboard, int whiteStonesToPlace, int blackStonesToPlace, int playerToMove, bool onlyStoneTaking)
{
    try {
        lastError = nullptr;
        return GetBestMove(whiteBitboard, blackBitboard, whiteStonesToPlace, blackStonesToPlace, playerToMove, onlyStoneTaking);
    } catch (std::exception& ex) {
        lastError = std::current_exception();
        return 0;
    }
}

std::string MalomSolutionAccess::GetLastError()
{
    if (!lastError) {
        return "No error";
    }
    try {
        std::rethrow_exception(lastError);
    } catch (const std::exception& ex) {
        return ex.what();
    }
}

int MalomSolutionAccess::GetBestMoveStr(std::string args)
{
    try {
        std::vector<std::string> argsSplit;
        std::stringstream ss(args);
        std::string arg;
        while (std::getline(ss, arg, ' ')) {
            argsSplit.push_back(arg);
        }
        const int numArgs = 6;
        if (argsSplit.size() != numArgs) {
            throw std::invalid_argument("Invalid number of arguments after splitting the string.");
        }
        return GetBestMove(std::stoi(argsSplit[0]), std::stoi(argsSplit[1]), std::stoi(argsSplit[2]), std::stoi(args[3]), std::stoi(argsSplit[4]), argsSplit[5] != "0");
    } catch (std::exception& ex) {
        std::cerr << "Fatal exception: " << ex.what() << std::endl;
        return 0;
    }
}

void MalomSolutionAccess::InitializeIfNeeded()
{
    if (pp != nullptr) {
        return;
    }
    InitRules(); // Implementation missing
    SetVariantStripped();
    if (!Sectors.HasDatabase) // Implementation missing
    {
        throw std::runtime_error("Database files not found in the current working directory");
    }
    pp = std::make_shared<PerfectPlayer>();
}

void MalomSolutionAccess::MustBeBetween(std::string paramName, int value, int min, int max)
{
    if (value < min || value > max) {
        throw std::out_of_range(paramName + " must be between " + std::to_string(min) + " and " + std::to_string(max));
    }
}

void MalomSolutionAccess::SetVariantStripped()
{
    // copy-paste from Rules.vb, but references to Main stripped

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
