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

#include "stdafx.h"
#include "sec_val.h"

map<id, sec_val> sec_vals; // vigyazat: STONE_DIFF esetben vannak benne nagyon
                           // nem letezo szektorok is
#ifndef STONE_DIFF
map<sec_val, id> inv_sec_vals;
#endif
sec_val virt_loss_val = 0, virt_win_val = 0;

void init_sec_vals()
{
#ifdef DD
#ifndef STONE_DIFF
    FILE *f;
    fopen_s(&f, sec_val_fname.c_str(), "rt");
    if (!f)
        failwith(VARIANT_NAME ".secval file not found.");
    fscanf_s(f, "virt_loss_val: %hd\nvirt_win_val: %hd\n", &virt_loss_val,
             &virt_win_val);
    assert(virt_win_val == -virt_loss_val);
    int n;
    fscanf_s(f, "%d\n", &n);
    for (int i = 0; i < n; i++) {
        int w, b, wf, bf, v;
        fscanf_s(f, "%d %d %d %d  %d\n", &w, &b, &wf, &bf, &v);
        sec_vals[id(w, b, wf, bf)] = v;
    }
    fclose(f);
#else
    for (int W = 0; W <= max_ksz; W++) {
        for (int WF = 0; WF <= max_ksz; WF++) {
            for (int B = 0; B <= max_ksz; B++) {
                for (int BF = 0; BF <= max_ksz; BF++) {
                    id s = id {W, WF, B, BF};
                    sec_vals[s] = s.W + s.WF - s.B - s.BF;
                }
            }
        }
    }
    virt_win_val = max_ksz + 1;
    virt_loss_val = -max_ksz - 1;
#endif
    // azert kell, mert egyreszt korrigalas, masreszt levonunk belole egyet a
    // gui_eval_elem2-ben a KLE-s szektorok ertekenel (a -5 csak biztonsagi,
    // lehet, hogy eleg lenne -1 is)
    assert(2 * virt_loss_val - 5 > sec_val_min_value);
#else
    virt_loss_val = -1;
    virt_win_val = 1;
#endif

#ifndef STONE_DIFF
    for (auto sv : sec_vals) {
        if (sv.second) { // not NTREKS if DD  (if not DD, then only the virt
                         // sectors (which btw don't get here) are non-0)
            assert(!inv_sec_vals.count(sv.second)); // non-NTREKS sec_vals
                                                    // should be unique
            inv_sec_vals[sv.second] = id(sv.first);
        }
    }
#endif

#ifdef HAS_SECTOR_GRAPH
    for (auto s : sector_list) {
        assert(sec_vals.count(s)); // every sector has a value
        auto xx = sec_vals[s];
        assert(s.transient() || sec_vals[s] == -sec_vals[-s]); // wus are
                                                               // zero-sum
    }
#endif
}

string sec_val_to_sec_name(sec_val v)
{
    if (v == 0)
#ifdef DD
#ifdef STONE_DIFF
        return "0";
#else
        return "NTESC";
#endif
#else
        return "D";
#endif
    else if (v == virt_loss_val)
        return "L";
    else if (v == virt_win_val)
        return "W";
    else {
#ifdef STONE_DIFF
        return to_string(v);
#else
        assert(inv_sec_vals.count(v));
        return to_string(v) + " (" + inv_sec_vals[v].to_string() + ")";
#endif
    }
}