//#pragma once

#include "windowsX.h"
#include "windows.h"

#define indent 32
#define field_size 300
#define cell_size (field_size / 10)
#define button_height 25
#define field2leftindent (12*indent + field_size)

enum CellTypes
{
	MISS = 124,
	EMPTY_CELL = 125,
	SHIP_IN_CELL = 126
};


bool ClickProc(int x, int y, int &cx, int &cy, LPARAM lParam);
void SB_InitOnce();


typedef void(_cdecl* SB_LClickProc)(LPARAM, HWND);
typedef void(_cdecl* SB_Paint)(HDC);

typedef __int32 cell_type;

struct IOFuns
{
	SB_Paint PaintFun;
	SB_LClickProc LCProcFun;
};

IOFuns iofuns;

cell_type SB_cells[2][10][10];
cell_type(*cells)[10];

bool fill_flag;