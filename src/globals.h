#pragma once
#ifndef _GLOBALS_H_
#define _GLOBALS_H_
#include "framework.h"
constexpr auto MAX_LOADSTRING = 100;

//grid layout
constexpr auto CELL_SIZE = 20; // Size of each cell (width and height)
constexpr auto CELL_SPACING = 0; // Spacing between cells
constexpr auto GRID_SIZE = 9; // Number of cells in a row or column
constexpr auto X_OFFSET = 10; // Left margin
constexpr auto Y_OFFSET = 10; // Top margin

constexpr auto CLIENT_WIDTH = X_OFFSET * 4 + GRID_SIZE * CELL_SIZE + (GRID_SIZE - 1) * CELL_SPACING;
constexpr auto CLIENT_HEIGHT = (Y_OFFSET * 4 + GRID_SIZE * CELL_SIZE + (GRID_SIZE - 1) * CELL_SPACING + CELL_SIZE * 2) + 50;

constexpr DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;

typedef int PUZZLE_GRID[GRID_SIZE][GRID_SIZE];
#endif