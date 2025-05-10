#include "sudoku.h"
#include "assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>
#include <lua5.4/lauxlib.h>
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>

#define ZK_CELL(row, col) ((row) * ZK_SUDOKU_WIDTH + (col))

int lua_grid_get(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "sudoku");
    Sudoku* sdk = (Sudoku*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    int row = luaL_checkinteger(L, 2);
    int col = luaL_checkinteger(L, 3);

    if (row < 1 || row > ZK_SUDOKU_HEIGHT)
    {
        fprintf(stderr, "Invalid row (%d)!\n", row);
        exit(1);
    }

    if (col < 1 || col > ZK_SUDOKU_WIDTH)
    {
        fprintf(stderr, "Invalid column (%d)!\n", col);
        exit(1);
    }

    lua_pushinteger(L, sudoku_get_cell(sdk, row - 1, col - 1));

    return 1;
}

Sudoku* sudoku_create(void)
{
    Sudoku* sdk = malloc(sizeof(Sudoku));

    for (size_t row = 0; row < ZK_SUDOKU_HEIGHT; ++row)
    {
        for (size_t col = 0; col < ZK_SUDOKU_WIDTH; ++col)
        {
            sudoku_set_cell(sdk, row, col, 0);
        }
    }

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    lua_newtable(L);
    lua_setglobal(L, "rules");
    lua_newtable(L);
    lua_pushlightuserdata(L, sdk);
    lua_setfield(L, -2, "sudoku");
    lua_pushvalue(L, -1);
    lua_pushcclosure(L, lua_grid_get, 1);
    lua_setfield(L, -2, "get");
    lua_setglobal(L, "grid");

    sdk->L = L;
    sdk->rules = NULL;
    sdk->rules_size = 0;
    sdk->rules_capacity = 0;

    return sdk;
}

void sudoku_load_grid_from_str(Sudoku* sdk, const char* str)
{
    ZK_ASSERT(strlen(str) == ZK_SUDOKU_WIDTH * ZK_SUDOKU_HEIGHT);

    for (size_t row = 0; row < ZK_SUDOKU_HEIGHT; ++row)
    {
        for (size_t col = 0; col < ZK_SUDOKU_WIDTH; ++col)
        {
            sudoku_set_cell(sdk, row, col, str[ZK_CELL(row, col)] - '0');
        }
    }
}

static char* read_file(const char* filepath)
{
    FILE* f = fopen(filepath, "r");

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char* data = malloc(len + 1);
    size_t read = fread(data, 1, len, f);

    fclose(f);

    data[read] = '\0';

    return data;
}

static void sudoku_add_rule(Sudoku* sdk, Rule rule)
{
    ZK_ASSERT(sudoku_rule_is_unloaded(sdk, rule.name));

    if (sdk->rules_size >= sdk->rules_capacity)
    {
        sdk->rules_capacity = sdk->rules_capacity ? sdk->rules_capacity * 2 : 1;
        sdk->rules = realloc(sdk->rules, sizeof(Rule) * sdk->rules_capacity);
    }

    sdk->rules[sdk->rules_size++] = rule;
}

static void convert_json_object_to_lua(lua_State* L, cJSON* obj)
{
    lua_newtable(L);

    if (obj == NULL)
    {
        return;
    }

    size_t index = 1;
    cJSON* current_item = obj->child;
    while (current_item)
    {
        if (cJSON_IsArray(obj))
        {
            lua_pushinteger(L, index++);
        }
        else
        {
            lua_pushstring(L, current_item->string);
        }

        if (cJSON_IsNumber(current_item))
        {
            lua_pushinteger(L, current_item->valuedouble);
        }
        else if (cJSON_IsString(current_item))
        {
            lua_pushstring(L, current_item->valuestring);
        }
        else if (cJSON_IsBool(current_item))
        {
            lua_pushboolean(L, current_item->valueint);
        }
        else if (cJSON_IsArray(current_item) || cJSON_IsObject(current_item))
        {
            convert_json_object_to_lua(L, current_item);
        }
        else
        {
            lua_pushnil(L);
        }

        lua_settable(L, -3);

        current_item = current_item->next;
    }
}

static bool sudoku_rule_is_unloaded(const Sudoku* sdk, const char* rule)
{
    for (size_t i = 0; i < sdk->rules_size; ++i)
    {
        Rule* r = &sdk->rules[i];
        if (strcmp(r->name, rule) == 0)
        {
            return false;
        }
    }

    return true;
}

static int sudoku_load_rule(Sudoku* sdk, const char* rule, cJSON* data)
{
    lua_State* L = sdk->L;

    if (sudoku_rule_is_unloaded(sdk, rule))
    {
        char buffer[PATH_MAX];
        snprintf(buffer, sizeof(buffer), "./rules/%s.lua", rule);

        if (luaL_dofile(L, buffer) != LUA_OK)
        {
            fprintf(stderr, "Failed to load script %s: %s\n", rule, lua_tostring(L, -1));
            lua_pop(L, 1);
            return -1;
        }

        lua_getglobal(L, "rules");
        lua_pushstring(L, rule);
        lua_pushvalue(L, -3);
        lua_settable(L, -3);
        lua_pop(L, 2);
    }

    convert_json_object_to_lua(L, data);

    Rule r;
    r.name = strdup(rule);
    r.lua_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    sudoku_add_rule(sdk, r);

    return 0;
}

Sudoku* sudoku_from_file(const char* filepath)
{
    Sudoku* sdk = sudoku_create();

    char* data = read_file(filepath);

    cJSON* root = cJSON_Parse(data);

    if (!root)
    {
        fprintf(stderr, "JSON parse error at: %s\n", cJSON_GetErrorPtr());
        free(data);

        return sdk;
    }

    cJSON* grid_item = cJSON_GetObjectItemCaseSensitive(root, "grid");
    if (cJSON_IsString(grid_item) && (grid_item->valuestring != NULL))
    {
        sudoku_load_grid_from_str(sdk, grid_item->valuestring);
    }

    sudoku_load_rule(sdk, "std", NULL);

    cJSON* rules_item = cJSON_GetObjectItemCaseSensitive(root, "rules");
    if (cJSON_IsArray(rules_item))
    {
        cJSON* rule_item = rules_item->child;
        while (rule_item)
        {
            cJSON* name_item = cJSON_GetObjectItemCaseSensitive(rule_item, "name");
            cJSON* data_item = cJSON_GetObjectItemCaseSensitive(rule_item, "data");

            if (name_item && cJSON_IsString(name_item) && data_item && cJSON_IsObject(data_item))
            {
                sudoku_load_rule(sdk, name_item->valuestring, data_item);
            }

            rule_item = rule_item->next;
        }
    }

    cJSON_Delete(root);
    free(data);

    return sdk;
}

Cell sudoku_get_cell(const Sudoku* sdk, size_t row, size_t col)
{
    return sdk->grid[ZK_CELL(row, col)];
}

void sudoku_set_cell(Sudoku* sdk, size_t row, size_t col, Cell cell)
{
    sdk->grid[ZK_CELL(row, col)] = cell;
}

void sudoku_clear_cell(Sudoku* sdk, size_t row, size_t col)
{
    sudoku_set_cell(sdk, row, col, 0);
}

static bool sudoku_find_empty_cell(const Sudoku* sdk, size_t* row, size_t* col)
{
    for (*row = 0; *row < ZK_SUDOKU_HEIGHT; ++*row)
    {
        for (*col = 0; *col < ZK_SUDOKU_WIDTH; ++*col)
        {
            if (sudoku_get_cell(sdk, *row, *col) == 0)
            {
                return true;
            }
        }
    }

    return false;
}

static bool sudoku_is_valid(const Sudoku* sdk, size_t row, size_t col, Cell cell)
{
    lua_State* L = sdk->L;

    for (size_t i = 0; i < sdk->rules_size; ++i)
    {
        const Rule* rule = &sdk->rules[i];

        lua_getglobal(L, "rules");

        lua_getfield(L, -1, rule->name);
        if (!lua_istable(L, -1))
        {
            fprintf(stderr, "Rule %s not found!\n", rule->name);
            lua_pop(L, 2);
            return false;
        }

        lua_getfield(L, -1, "validate");
        if (!lua_isfunction(L, -1))
        {
            fprintf(stderr, "Rule '%s' has no validate() function defined!\n", rule->name);
            lua_pop(L, 3);
            return false;
        }

        lua_rawgeti(L, LUA_REGISTRYINDEX, rule->lua_ref);
        lua_pushinteger(L, row + 1);
        lua_pushinteger(L, col + 1);
        lua_pushinteger(L, cell);

        if (lua_pcall(L, 4, 1, 0) != LUA_OK)
        {
            fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 3);
            return false;
        }

        bool valid = lua_toboolean(L, -1);
        lua_pop(L, 3);

        if (!valid)
        {
            return false;
        }
    }

    return true;
}

static bool sudoku_find_most_constrained_empty_cell(const Sudoku* sdk, size_t* row, size_t* col)
{
    size_t num_candidates = 9;
    size_t candidate_row = 0;
    size_t candidate_col = 0;
    if (!sudoku_find_empty_cell(sdk, &candidate_row, &candidate_col))
    {
        return false;
    }

    for (*row = candidate_row; *row < ZK_SUDOKU_HEIGHT; ++*row)
    {
        for (*col = candidate_col; *col < ZK_SUDOKU_WIDTH; ++*col)
        {
            if (sudoku_get_cell(sdk, *row, *col) == 0)
            {
                size_t candidates = 0;
                for (size_t cell = 1; cell <= 9; ++cell)
                {
                    if (sudoku_is_valid(sdk, *row, *col, cell))
                    {
                        ++candidates;
                    }
                }

                if (candidates < num_candidates)
                {
                    num_candidates = candidates;
                    candidate_row = *row;
                    candidate_col = *col;
                }
            }
        }
    }

    *row = candidate_row;
    *col = candidate_col;

    return true;
}

bool sudoku_solve(Sudoku* sdk)
{
    size_t row, col;

    if (!sudoku_find_most_constrained_empty_cell(sdk, &row, &col))
        return true;

    for (Cell cell = 1; cell <= 9; ++cell)
    {
        if (sudoku_is_valid(sdk, row, col, cell))
        {
            sudoku_set_cell(sdk, row, col, cell);
            if (sudoku_solve(sdk))
            {
                return true;
            }

            sudoku_clear_cell(sdk, row, col);
        }
    }

    return false;
}

void sudoku_print(const Sudoku* sdk)
{
    for (size_t row = 0; row < ZK_SUDOKU_HEIGHT; ++row)
    {
        if (row % 3 == 0)
        {
            printf("+-------+-------+-------+\n");
        }

        for (size_t col = 0; col < ZK_SUDOKU_WIDTH; ++col)
        {
            if (col % 3 == 0)
            {
                printf("| ");
            }

            Cell cell = sudoku_get_cell(sdk, row, col);
            if (cell == 0)
            {
                printf(". ");
            } else
            {
                printf("%d ", cell);
            }
        }

        printf("|\n");
    }

    printf("+-------+-------+-------+\n");
}
