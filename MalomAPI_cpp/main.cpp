// You have to set the working directory to the directory of the database.

// #define no_init_all // Was needed only in VS 2017

#define USE_DEPRECATED_CLR_API_WITHOUT_WARNING

#include <cstdio>
#include <sstream>
#include <string>

#include "common.h"
#include "MalomSolutionAccess.h"

int main(int argc, char* argv[])
{
    if (argc == 2) {
        sec_val_path = argv[1];
    }

    //int res = MalomSolutionAccess::getBestMove(0, 0, 9, 9, 0, false);
    int res = MalomSolutionAccess::getBestMove(1, 2, 8, 8, 0, false); // Correct output: 16384
    //int res = MalomSolutionAccess::getBestMove(1 + 2 + 4, 8 + 16 + 32, 100, 0, 0, false); // tests exception
    // int res = MalomSolutionAccess::getBestMove(1 + 2 + 4, 1 + 8 + 16 + 32, 0, 0, 0, false); // tests exception
    // int res = MalomSolutionAccess::getBestMove(1 + 2 + 4, 8 + 16 + 32, 0, 0, 0, true); // Correct output: any of 8, 16, 32

    printf("GetBestMove result: %d\n", res);

#ifdef _WIN32
    system("pause");
#endif 

    return 0;
}
