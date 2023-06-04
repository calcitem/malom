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
#include "common.h"

wstring str2wstr(const string &s)
{
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(s.c_str());
}

void failwith(string s)
{
    MessageBox(NULL, str2wstr(s).c_str(), str2wstr(VARIANT_NAME).c_str(),
               MB_ICONERROR);
    exit(7);
}
