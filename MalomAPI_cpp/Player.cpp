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

#include <memory> // for std::shared_ptr

class Player {
protected:
    std::shared_ptr<Game> G; // Assuming Game is a pre-defined class

public:
    // The object is informed to enter the specified game
    virtual void Enter(std::shared_ptr<Game> _g)
    {
        G = _g;
    }

    // The object is informed to exit from the game
    virtual void Quit()
    {
        if (G == nullptr)
            return;
        G = nullptr;
    }

    // The object is informed that it is its turn to move
    virtual void ToMove(GameState& s) = 0; // Assuming GameState is a pre-defined class

    // Notifies about the opponent's move
    virtual void FollowMove(const Object& M) { } // Assuming Object is a pre-defined class or built-in type

    // The object is informed that it is the opponent's turn to move
    virtual void OppToMove(GameState& s) { }

    // Game is over
    virtual void Over(GameState& s) { }

    // Cancel thinking
    virtual void CancelThinking() { }

    // Determine the opponent player
protected:
    Player* Opponent()
    {
        return (G->Ply(0).get() == this) ? G->Ply(1).get() : G->Ply(0).get(); // Assuming Game has a Ply function
    }
};
