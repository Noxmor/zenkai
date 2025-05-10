#include <stdio.h>
#include <stdlib.h>

#include "sudoku.h"

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        Sudoku* sdk = sudoku_from_file(argv[i]);

        printf("Sudoku:\n");
        sudoku_print(sdk);

        if (sudoku_solve(sdk))
        {
            printf("Solution:\n");
            sudoku_print(sdk);
        }
        else
        {
            printf("Unsolvable sudoku.\n");
        }

        free(sdk);
    }

    return 0;
}
