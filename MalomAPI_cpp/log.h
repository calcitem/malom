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

#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#ifndef WRAPPER
	struct Log{ //ez azert nincs a masik agban, mert a wrapper projektben nincs benne a log.cpp (de amugy semmi akadalya nem lenne belerakni)
		static bool log_to_file;
		static FILE *logfile;
		static void setup_logfile(string fname, string extension);
		static string fname, fnamelogging, donefname;
		static void close();
	};
#endif


template <typename... Args>
    void LOG(const char* format, Args... args)
    {
#ifndef WRAPPER
#if defined(_WIN32)
            printf_s(format, args...);
            fflush(stdout);
            if (Log::log_to_file) {
                fprintf_s(Log::logfile, format, args...);
                fflush(Log::logfile);
            }
#elif defined(__linux__)
            printf(format, args...);
            fflush(stdout);
            if (Log::log_to_file) {
                fprintf(Log::logfile, format, args...);
                fflush(Log::logfile);
            }
#endif
#else
            char buf[255];
            sprintf_s(buf, format, args...);
            System::Diagnostics::Debug::Write(gcnew System::String(buf));
#endif
    }

#endif // LOG_H_INCLUDED
