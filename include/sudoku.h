#ifndef ZK_SUDOKU_H
#define ZK_SUDOKU_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define ZK_SUDOKU_WIDTH 9
#define ZK_SUDOKU_HEIGHT 9

typedef struct lua_State lua_State;

typedef uint8_t Cell;

typedef struct Rule
{
    const char* name;
    int lua_ref;
} Rule;

typedef struct Sudoku
{
    Cell grid[ZK_SUDOKU_WIDTH * ZK_SUDOKU_HEIGHT];
    lua_State* L;
    Rule* rules;
    size_t rules_size;
    size_t rules_capacity;
} Sudoku;

Sudoku* sudoku_create(void);

void sudoku_load_grid_from_str(Sudoku* sdk, const char* str);

Sudoku* sudoku_from_file(const char* filepath);

Cell sudoku_get_cell(const Sudoku* sdk, size_t row, size_t col);

void sudoku_set_cell(Sudoku* sdk, size_t row, size_t col, Cell cell);

void sudoku_clear_cell(Sudoku* sdk, size_t row, size_t col);

bool sudoku_solve(Sudoku* sdk);

void sudoku_print(const Sudoku* sdk);

#endif
