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

#pragma once

#include "eval_elem.h"
#include "sector_graph.h"
#include "sec_val.h"

#ifndef WRAPPER
#include "movegen.h"
#endif

class Hash;
class Sector;

#ifndef WRAPPER
struct short_id
{
    char sid;

    short_id();
    short_id(char sid);
    short_id(Sector *s);
    operator Sector *();
    operator id();

    bool operator==(const short_id &o) const { return sid == o.sid; }
    bool operator>=(const short_id &o) const { return sid >= o.sid; }
};
#endif

class Sector
{
    char fname[255];

#ifndef WRAPPER
    bool file_existed;

    void init();

    unsigned char *eval;
#endif
    int eval_size;

    map<int, int> em_set;

#ifdef WRAPPER
    FILE *f;
#endif

#ifdef DD
    static const int header_size = 64;
#else
    static const int header_size = 0;
#endif
    void read_header(FILE *f);
    void write_header(FILE *f);
    void read_em_set(FILE *f);

public:
    Hash *hash;

    int W, B, WF, BF;
    id id;

    Sector(::id id);

    eval_elem2 get_eval(int i);
    eval_elem_sym2 get_eval_inner(int i);

#ifdef DD
    pair<sec_val, field2_t> extract(int i);
    void intract(int i, pair<sec_val, field2_t> x);
#endif

#ifndef WRAPPER
    char sid; // a bucketek altal hasznalt id

    signed char wms;

#ifdef DD
    void set_eval(int i, eval_elem2 r);
    void set_eval_inner(int h, eval_elem_sym2 r);
#else
    void set_eval(int i, eval_elem2 r);
    void set_eval_inner(int h, eval_elem_sym r);
    void set_eval_inner(int h, eval_elem_sym2 r);
#endif

    void save();

    void check_consis();
#endif

    // statisztikak:
    int max_val, max_count;

#ifndef WRAPPER
    void allocate(bool doinit, ::id parent = id::null());
    void release();
#endif

    void allocate_hash();
    void release_hash();

public:
    sec_val sval;
};

extern Sector *sectors[max_ksz + 1][max_ksz + 1][max_ksz + 1][max_ksz + 1];
#define sectors(id) (sectors[(id).W][(id).B][(id).WF][(id).BF])

extern vector<Sector *> sector_objs;
