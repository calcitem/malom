/*
Malom, a Nine Men's Morris (and variants) player and solver program.
Copyright(C) 2007-2016  Gabor E. Gevay, Gabor Danner

See our webpage (and the paper linked from there):
http://compalg.inf.elte.hu/~ggevay/mills/index.php


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef WRAPPER_H_INCLUDED
#define WRAPPER_H_INCLUDED

#include "common.h"
#include "hash.h"
#include "symmetries.h"
#include "debug.h"
#include "sector_graph.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <cmath> // for factorial function
#include <cassert>
#include <iostream>
#include <set>
#include <map>
#include <tuple>

using namespace std;



namespace Wrappers {

class WSector;

#include <cmath> // for factorial function
#include <string>
#include <unordered_map>

// Note: Assuming 'board' and 'sec_val' are defined elsewhere in your code

struct eval_elem {
    enum class cas { val,
        count,
        sym };
    cas c;
    int x;

    eval_elem(cas c, int x)
        : c(c)
        , x(x)
    {
    }

    // Assuming ::eval_elem is a type defined in your code
    eval_elem(::eval_elem e)
        : c(static_cast<cas>(e.c))
        , x(e.x)
    {
    }
};

struct gui_eval_elem2; // Assuming this struct is defined elsewhere

class WSector {
public:
    ::Sector* s;
    WSector(id id)
        : s(new ::Sector(id.tonat()))
    {
    }

    std::pair<int, Wrappers::gui_eval_elem2> hash(board a);

    sec_val sval() { return s->sval; }
};

struct gui_eval_elem2 {
private:
    sec_val key1;
    int key2;
    Sector* s;
    enum class Cas { Val,
        Count };

    eval_elem2 to_eval_elem2() const
    {
        return eval_elem2 { key1, key2 };
    }

public:
    gui_eval_elem2(sec_val key1, int key2, Sector* s)
        : key1 { key1 }
        , key2 { key2 }
        , s { s }
    {
    }
    gui_eval_elem2(::eval_elem2 e, Sector* s)
        : gui_eval_elem2 { e.key1, e.key2, s }
    {
    }
    inline static const bool ignore_DD = false;

    gui_eval_elem2 undo_negate(WSector* s)
    {
        auto a = this->to_eval_elem2().corr((s ? s->sval() : virt_unique_sec_val()) + (this->s ? this->s->sval : virt_unique_sec_val()));
        a.key1 *= -1;
        if (s)
            a.key2++;
        return gui_eval_elem2(a, s ? s->s : nullptr);
    }

    static sec_val abs_min_value()
    {
        assert(::virt_loss_val != 0);
        return ::virt_loss_val - 2;
    }

    static void drop_DD(eval_elem2& e)
    {
        assert(e.key1 >= abs_min_value());
        assert(e.key1 <= ::virt_win_val);
        assert(e.key1 != ::virt_loss_val - 1);
        if (e.key1 != virt_win_val && e.key1 != ::virt_loss_val && e.key1 != abs_min_value())
            e.key1 = 0;
    }

    int compare(const gui_eval_elem2& o) const
    {
        assert(s == o.s);
        if (!ignore_DD) {
            if (key1 != o.key1)
                return key1 < o.key1 ? -1 : 1;
            else if (key1 < 0)
                return key2 < o.key2 ? -1 : 1;
            else if (key1 > 0)
                return key2 > o.key2 ? -1 : 1;
            else
                return 0;
        } else {
            auto a1 = to_eval_elem2().corr(s ? s->sval : virt_unique_sec_val());
            auto a2 = o.to_eval_elem2().corr(o.s ? o.s->sval : virt_unique_sec_val());
            drop_DD(a1);
            drop_DD(a2);
            if (a1.key1 != a2.key1)
                return a1.key1 < a2.key1 ? -1 : 1;
            else if (a1.key1 < 0)
                return a1.key2 < a2.key2 ? -1 : 1;
            else if (a1.key1 > 0)
                return a2.key2 < a1.key2 ? -1 : 1;
            else
                return 0;
        }
    }

    bool operator<(const gui_eval_elem2& b) const { return this->compare(b) < 0; }
    bool operator>(const gui_eval_elem2& b) const { return this->compare(b) > 0; }
    bool operator==(const gui_eval_elem2& b) const { return this->compare(b) == 0; }

    static gui_eval_elem2 min_value(WSector* s)
    {
        return gui_eval_elem2 { static_cast<sec_val>(abs_min_value() - (s ? s->sval() : virt_unique_sec_val())), 0, s ? s->s : nullptr };
    }

    static gui_eval_elem2 virt_loss_val()
    {
        assert(::virt_loss_val);
        return gui_eval_elem2 { static_cast<sec_val>(::virt_loss_val - virt_unique_sec_val()), 0, nullptr };
    }

    static sec_val virt_unique_sec_val()
    {
        assert(::virt_loss_val);
#ifdef DD
        return ::virt_loss_val - 1;
#else
        return 0;
#endif
    }

    sec_val akey1()
    {
        return key1 + (s ? s->sval : virt_unique_sec_val());
    }

    std::string toString()
    {
        assert(::virt_loss_val);
        assert(::virt_win_val);
        std::string s1, s2;

        sec_val akey1 = this->akey1();
        s1 = sec_val_to_sec_name(akey1);

        if (key1 == 0)
#ifdef DD
            s2 = "C";
#else
            s2 = "";
#endif
        else
            s2 = std::to_string(key2);

#ifdef DD
        return s1 + ", (" + std::to_string(key1) + ", " + s2 + ")";
#else
        return s1 + s2;
#endif
    }
};

class Nwu {
public:
    static std::vector<id> WuIds;
    static void initWuGraph()
    {
        init_sector_graph();
        WuIds = std::vector<id>();
        for (auto it = wu_ids.begin(); it != wu_ids.end(); ++it)
            WuIds.push_back(id(*it));
    }
    static std::vector<id> wuGraphT(id u)
    {
        auto r = std::vector<id>();
        wu* w = wus[u.tonat()];
        for (auto it = w->parents.begin(); it != w->parents.end(); ++it)
            r.push_back(id((*it)->id));
        return r;
    }
    static bool twine(id w)
    {
        return wus[w.tonat()]->twine;
    }
};

class Init {
public:
    static void init_sym_lookuptables()
    {
        ::init_sym_lookuptables();
    }
    static void init_sec_vals()
    {
        ::init_sec_vals();
    }
};

class Constants {
public:
    static const int variant = VARIANT;
    inline static const std::string fname_suffix = FNAME_SUFFIX;
    const std::string movegenFname = movegen_file;

    enum class Variants {
        std = STANDARD,
        mora = MORABARABA,
        lask = LASKER
    };

#ifdef DD
    static const bool dd = true;
#else
    static const bool dd = false;
#endif

    static const bool FBD = FULL_BOARD_IS_DRAW;

#ifdef FULL_SECTOR_GRAPH
    static const bool extended = true;
#else
    static const bool extended = false;
#endif
};

class Helpers {
public:
    static std::string toclp(board a)
    {
        return ::toclp(a);
    }
};
}

#endif // WRAPPER_H_INCLUDED
