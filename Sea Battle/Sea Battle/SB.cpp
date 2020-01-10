/*---------------- This section is common with: Sea battle.cpp ----------------*/

#include "windowsX.h"
#include "windows.h"
#include "utility"

#define indent 32
#define field_size 300
#define cell_size (field_size / 10)
#define button_height 25
#define field2leftindent (12*indent + field_size)

enum CellTypes
{
	EMPTY_CELL = 124,
	MISS = 125,
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
cell_type bot_pl_cells[100];

bool fill_flag;

/*---------------------       Section end       ------------------*/



/*--------------------- Section: SB global vars ------------------*/

#include "locale.h"
#include "stdint.h"
#include "intrin.h"

enum ButtonType
{
	LOC_COM = 0,
	LOC_OPEN,
	BOT,
	BUTTON_COUNT
};

void BotMove();
void SB_draw_field(HDC hdc, int left, int top);

void SB_LClickProcChoosing(LPARAM lParam, HWND hWnd);
void SB_LClickProcBattleLocCom(LPARAM lParam, HWND hWnd);
void SB_LClickProcFilling(LPARAM lParam, HWND hWnd);

void SB_PaintChoosing(HDC hdc);
void SB_PaintFilling(HDC hdc);
void SB_PaintBattleLocCom(HDC hdc);
void SB_PaintBattleBot(HDC hdc);

typedef void(_cdecl* SB_void)(void);

struct Point
{
	int16_t x, y;
};

struct Button
{
	int16_t x1, x2, y1, len;
	const char *str;
};

struct Ship
{
	int16_t x, y, dead_flag;
};

struct BotHunting
{
	__int32 count;
	Point p[4];
};

SB_void bot_move_fun;

Ship SB_ShipCells[2][4][8];
Ship(*shipcells)[8];

Button SB_buttons[BUTTON_COUNT];
Button ok;

BotHunting bot_hunt;
bool player_flag;
bool open_flag;
bool bot_flag;
__int32 dead_ships_counter[2];
int bot_count;
char player_str[] = "Игрок ";

/*---------------------      Section end        ------------------*/


void _fastcall SB_memset16allign(__m128* dst, const __m128 val, size_t count)
{
	for (int i = 0; i < count; i++)
		dst[i] = val;
}
void SB_init()
{
	cells = SB_cells[0];
	iofuns.PaintFun = SB_PaintChoosing;
	iofuns.LCProcFun = SB_LClickProcChoosing;
	shipcells = SB_ShipCells[0];
	player_flag = 0;
	fill_flag = 0;
	bot_flag = 0;
	player_str[6] = '1';
	bot_hunt.p[0].x = EMPTY_CELL;

	*(__int64*)dead_ships_counter = (__int64)0;
	__int32 t1[4] = { 0, 0, 0, 0 };
	SB_memset16allign((__m128*)SB_ShipCells, *(__m128*)t1, sizeof(Ship) * 2 * 4 * 8 / 16);
	cell_type t[4] = { EMPTY_CELL, EMPTY_CELL, EMPTY_CELL, EMPTY_CELL };
	SB_memset16allign((__m128*)SB_cells, *(__m128*)t, sizeof(cell_type) * 2 * 10 * 10 / 16);
}
void SB_InitOnce()
{
	ok.x1 = 2 * indent + field_size;
	ok.x2 = 2 * indent + field_size + 55;
	ok.y1 = indent;
	ok.len = 6;
	ok.str = "Готово";

	SB_buttons[LOC_COM].x1 = indent;
	SB_buttons[LOC_COM].x2 = 6 * indent;
	SB_buttons[LOC_COM].y1 = indent;
	SB_buttons[LOC_COM].len = 11;
	SB_buttons[LOC_COM].str = "Общий экран";

	SB_buttons[LOC_OPEN].x1 = indent;
	SB_buttons[LOC_OPEN].x2 = 6 * indent;
	SB_buttons[LOC_OPEN].y1 = 2 * indent + button_height;
	SB_buttons[LOC_OPEN].len = 17;
	SB_buttons[LOC_OPEN].str = "Разделенный экран";

	SB_buttons[BOT].x1 = 7 * indent;
	SB_buttons[BOT].x2 = 13 * indent;
	SB_buttons[BOT].y1 = indent;
	SB_buttons[BOT].len = 14;
	SB_buttons[BOT].str = "Одиночная игра";

	setlocale(0, "");
	SB_init();
}

bool ClickProc(int x, int y, int &cx, int &cy, LPARAM lParam)
{
	y -= indent;

	if (x < 0 || y < 0 || x >= field_size || y >= field_size)
		return false;

	cx = x / cell_size, cy = y / cell_size;

	return true;
}

bool SB_ButtonClick(int x, int y, Button &but)
{
	if (x >= but.x1  &&  x <= but.x2   &&   y >= but.y1  &&  y <= but.y1 + button_height)
		return true;
	return false;
}

void SB_ChangePlayer()
{
	player_flag = !player_flag;
	shipcells = SB_ShipCells[player_flag];
	cells = SB_cells[player_flag];
}

void SB_LClickProcChoosing(LPARAM lParam, HWND hWnd)
{
	int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
	if (SB_ButtonClick(x, y, SB_buttons[LOC_COM]))
	{
		goto mode_choosen;
	}
	if (SB_ButtonClick(x, y, SB_buttons[LOC_OPEN]))
	{
		open_flag = 1;
		goto mode_choosen;
	}
	if (SB_ButtonClick(x, y, SB_buttons[BOT]))
	{
		player_str[6] = 0;
		bot_flag = 1;
		goto mode_choosen;
	}
	return;
mode_choosen:
	fill_flag = 1;
	iofuns.LCProcFun = SB_LClickProcFilling;
	iofuns.PaintFun = SB_PaintFilling;
	InvalidateRect(hWnd, 0, true);
}

void GenBotField()
{
	//1
}

bool CheckField()
{
	cell_type a[10][10];
	int ships[4] = { 0 };
	memcpy(a, cells, sizeof(cell_type) * 10 * 10);
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (a[i][j] == SHIP_IN_CELL)
			{
				int *t;
				if (a[i + 1][j] == SHIP_IN_CELL)
					t = &i;
				else
					t = &j;
				int k = -1;
				do
				{
					k++;
					(*t)++;
				} while ((*t) < 10 && a[i][j] == SHIP_IN_CELL);
				if (k > 3 || ships[k] > 3 - k)
					return false;
				int t2 = ships[k] * (k + 1);
				ships[k]++;
				for (int it = t2; it <= t2 + k; it++)
				{
					(*t)--;
					a[i][j] = k * 8 + it;
					shipcells[k][it].x = i;
					shipcells[k][it].y = j;
				}
			}
		}
	}
	for (int i = 0; i < 4; i++)
	{
		if (ships[i] != 4 - i)
			return false;
	}
	memcpy(cells, a, sizeof(cell_type) * 10 * 10);
	return true;
}
void SB_LClickProcFilling(LPARAM lParam, HWND hWnd)
{
	int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
	if (SB_ButtonClick(x, y, ok))
	{
		if (!CheckField())
		{
			MessageBoxA(hWnd, "Корабли расставлены не верно", "Ошибка", MB_OK);
			return;
		}
		if (bot_flag)
		{
			bot_count = 100;
			fill_flag = 0;
			SB_ChangePlayer();
			GenBotField();
			bot_move_fun = BotMove;
			for (int i = 0; i < 100; i++)
			{
				bot_pl_cells[i] = i;
			}
			iofuns.LCProcFun = SB_LClickProcBattleLocCom;
			iofuns.PaintFun = SB_PaintBattleBot;
		}
		if (player_flag)
		{
			iofuns.LCProcFun = SB_LClickProcBattleLocCom;
			iofuns.PaintFun = SB_PaintBattleLocCom;
			fill_flag = 0;
		}
		else
		{
			SB_ChangePlayer();
			player_str[6] = '2';
		}
		InvalidateRect(hWnd, 0, true);
		return;
	}
	int cx, cy;
	x -= indent;
	if (!ClickProc(x, y, cx, cy, lParam))
		return;
	if (cx + 1 < 10)
	{
		if (cy + 1 < 10)
			if (cells[cx + 1][cy + 1] != EMPTY_CELL)
				return;
		if (cy - 1 >= 0)
			if (cells[cx + 1][cy - 1] != EMPTY_CELL)
				return;
	}
	if (cx - 1 >= 0)
	{
		if (cy + 1 < 10)
			if (cells[cx - 1][cy + 1] != EMPTY_CELL)
				return;
		if (cy - 1 >= 0)
			if (cells[cx - 1][cy - 1] != EMPTY_CELL)
				return;
	}
	cells[cx][cy] = SHIP_IN_CELL;
	InvalidateRect(hWnd, 0, true);
}


void SB_cells_misslineX(int y, int x1, int x2)
{
	x1--;
	x2++;
	if (y >= 0 && y <= 9)
		for (; x1 <= x2; x1++)
			if (x1 >= 0 && x1 <= 9)
				cells[x1][y] = MISS;
}
void SB_cells_misslineY(int x, int y1, int y2)
{
	y1--;
	y2++;
	if (x >= 0 && x <= 9)
		for (; y1 <= y2; y1++)
			if (y1 >= 0 && y1 <= 9)
				cells[x][y1] = MISS;
}
void SB_LClickProcBattleLocCom(LPARAM lParam, HWND hWnd)
{
	int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
	if (player_flag)
		x -= field2leftindent;
	else
		x -= indent;
	int cx, cy;
	if (!ClickProc(x, y, cx, cy, lParam) || cells[cx][cy] == MISS)
		return;
	int _ship = cells[cx][cy];
	if (_ship == EMPTY_CELL)
	{
		RECT r;
		r.left = 0;
		r.top = 0;
		r.bottom = 16;
		r.right = 1100;
		InvalidateRect(hWnd, &r, true);
		InvalidateRect(hWnd, 0, false);
		cells[cx][cy] = MISS;
		SB_ChangePlayer();
		player_flag = 0;
		return;
	}
	if (shipcells[0][_ship].dead_flag == 0)
	{
		InvalidateRect(hWnd, 0, false);
		shipcells[0][_ship].dead_flag = 1;
		int i = _ship / 8;
		int j = _ship - (_ship % 8 % (i + 1));
		for (int t = j; t <= i + j; t++)
		{
			if (shipcells[0][t].dead_flag == 0)
				return;
		}
		dead_ships_counter[player_flag]++;
		shipcells[0][j].dead_flag = 2;
		SB_cells_misslineX(shipcells[0][j + i].y - 1, shipcells[0][j + i].x, shipcells[0][j].x);
		SB_cells_misslineX(shipcells[0][j].y + 1, shipcells[0][j + i].x, shipcells[0][j].x);
		SB_cells_misslineY(shipcells[0][j + i].x - 1, shipcells[0][j + i].y, shipcells[0][j].y);
		SB_cells_misslineY(shipcells[0][j].x + 1, shipcells[0][j + i].y, shipcells[0][j].y);
	}
}

void SB_PaintButton(HDC hdc, Button &but)
{
	Rectangle(hdc, but.x1, but.y1, but.x2, but.y1 + button_height);
	TextOutA(hdc, but.x1 + 4, but.y1 + 4, but.str, but.len);
}

void SB_PaintChoosing(HDC hdc)
{
	for (int i = 0; i < BUTTON_COUNT; i++)
		SB_PaintButton(hdc, SB_buttons[i]);
}

void SB_drawShipsFill(HDC hdc)
{
	for (int i = 0, x = indent + 1; i < 10; i++, x += cell_size)
	{
		for (int j = 0, y = indent + 1; j < 10; j++, y += cell_size)
		{
			if (cells[i][j] != EMPTY_CELL)
				Rectangle(hdc, x, y, x + cell_size - 1, y + cell_size - 1);
		}
	}
}
void SB_PaintFilling(HDC hdc)
{
	TextOutA(hdc, 0, 0, player_str, 7);
	SB_draw_field(hdc, indent, indent);
	SB_PaintButton(hdc, ok);
	SB_drawShipsFill(hdc);
}

void SB_Rectangle(HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, 0);
	LineTo(hdc, x1, y2);
	LineTo(hdc, x2, y2);
	LineTo(hdc, x2, y1);
	LineTo(hdc, x1, y1);
}

void SB_drawMisses(HDC hdc, int left, int top, cell_type lcells[10][10])
{
	int x = left + (cell_size / 2) - 1;
	for (int i = 0; i < 10; i++, x += cell_size)
	{
		int y = top + (cell_size / 2) - 1;
		for (int j = 0; j < 10; j++)
		{
			if (lcells[i][j] == MISS)
				Rectangle(hdc, x, y, x + 2, y + 2);
			y += cell_size;
		}
	}
}
void SB_drawShipsBatlle(HDC hdc, int left, int top, Ship lships[4][8], bool openfl)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < (4 - i)*(i + 1); j += i + 1)
		{
			if (openfl || lships[i][j].dead_flag == 2)
				SB_Rectangle(hdc, lships[i][j].x*cell_size + left + cell_size - 1, lships[i][j].y*cell_size + top + cell_size - 1,
					lships[i][j + i].x*cell_size + left + 1, lships[i][j + i].y*cell_size + top + 1);
			for (int k = j; k <= j + i; k++)
			{
				if (lships[i][k].dead_flag != 0)
				{
					int x = lships[i][k].x*cell_size + left, y = lships[i][k].y*cell_size + top;
					MoveToEx(hdc, x, y, 0);
					int x2 = x + cell_size, y2 = y + cell_size;
					LineTo(hdc, x2, y2);
					MoveToEx(hdc, x2, y, 0);
					LineTo(hdc, x, y2);
				}
			}
		}
	}
}
void SB_PaintBattleLocCom(HDC hdc)
{
	SB_draw_field(hdc, indent, indent);
	SB_draw_field(hdc, field2leftindent, indent);
	SB_drawMisses(hdc, indent, indent, SB_cells[0]);
	SB_drawMisses(hdc, field2leftindent, indent, SB_cells[1]);
	SB_drawShipsBatlle(hdc, indent, indent, SB_ShipCells[0], false);
	SB_drawShipsBatlle(hdc, field2leftindent, indent, SB_ShipCells[1], false);
	if (open_flag)
	{
		SB_draw_field(hdc, indent, 2 * indent + field_size);
		SB_draw_field(hdc, field2leftindent, 2 * indent + field_size);
		SB_drawMisses(hdc, indent, 2 * indent + field_size, SB_cells[1]);
		SB_drawMisses(hdc, field2leftindent, 2 * indent + field_size, SB_cells[0]);
		SB_drawShipsBatlle(hdc, indent, 2 * indent + field_size, SB_ShipCells[1], true);
		SB_drawShipsBatlle(hdc, field2leftindent, 2 * indent + field_size, SB_ShipCells[0], true);
	}
	int x;
	if (!player_flag)
		x = indent + field_size / 2 - 15;
	else
		x = field2leftindent + field_size / 2 - 15;
	TextOutA(hdc, x, 0, "Ход", 3);
	if (dead_ships_counter[player_flag] == 10)
	{
		char str[] = "Победил игрок 2\t";
		if (player_flag)
		{
			SB_drawShipsBatlle(hdc, indent, indent, SB_ShipCells[0], true);
		}
		else
		{
			SB_drawShipsBatlle(hdc, field2leftindent, indent, SB_ShipCells[1], true);
			str[14] = '1';
		}
		MessageBoxA(0, str, "WINNER WINNER CHICKEN DINNER!", MB_OK);
		InvalidateRect(0, 0, true);
		SB_init();
	}
}

void BotHunt2()
{

}

void shot(int x, int y)
{
	if (SB_cells[0][x][y] == EMPTY_CELL)
	{

	}
	else
	{

	}
}

void left()		{ shot(bot_hunt.p[0].x - 1, bot_hunt.p[0].y); }
void right()	{ shot(bot_hunt.p[0].x + 1, bot_hunt.p[0].y); }
void top()		{ shot(bot_hunt.p[0].x, bot_hunt.p[0].y - 1); }
void bottom()	{ shot(bot_hunt.p[0].x, bot_hunt.p[0].y + 1); }

void BotHunt1()
{
	int count = 0;
	SB_void funs[4];
	int x = bot_hunt.p[0].x, y = bot_hunt.p[0].y;
	if (x - 1 >= 0 && SB_cells[0][x - 1][y] != MISS)
	{
		funs[count] = left;
		count++;
	}
	if (x + 1 >= 0 && SB_cells[0][x + 1][y] != MISS)
	{
		funs[count] = right;
		count++;
	}
	if (y - 1 >= 0 && SB_cells[0][x][y - 1] != MISS)
	{
		funs[count] = top;
		count++;
	}
	if (y + 1 >= 0 && SB_cells[0][x][y + 1] != MISS)
	{
		funs[count] = bottom;
		count++;
	}
	int rand = (((int)__rdtsc()) >> 1) % count;
	funs[rand]();
}

void BotMove()
{
	int randind = (((int)__rdtsc()) >> 1) % bot_count;
	int rind = bot_pl_cells[randind];
	bot_count--;
	std::swap(bot_pl_cells[randind], bot_pl_cells[bot_count]);
	//TODO: check field
	cell_type _ship = SB_cells[0][0][rind];
	if (_ship == EMPTY_CELL)
	{
		SB_cells[0][0][rind] = MISS;
		player_flag = 1;
		return;
	}
	int i = _ship / 8;
	int j = _ship - (_ship % 8 % (i + 1));
	if (!i)
	{
		dead_ships_counter[0]++;
		SB_ShipCells[0][0][j].dead_flag = 2;
		player_flag = 1;
		//edit
		SB_cells_misslineX(SB_ShipCells[0][0][j].y - 1, SB_ShipCells[0][0][j].x, SB_ShipCells[0][0][j].x);
		SB_cells_misslineX(SB_ShipCells[0][0][j].y + 1, SB_ShipCells[0][0][j].x, SB_ShipCells[0][0][j].x);
		SB_cells_misslineY(SB_ShipCells[0][0][j].x - 1, SB_ShipCells[0][0][j].y, SB_ShipCells[0][0][j].y);
		SB_cells_misslineY(SB_ShipCells[0][0][j].x + 1, SB_ShipCells[0][0][j].y, SB_ShipCells[0][0][j].y);
		return;
	}
	SB_ShipCells[0][0][_ship].dead_flag = 1;
	bot_hunt.p[0].x = rind / 10;
	bot_hunt.p[0].y = rind % 10;
	bot_hunt.count = 1;
	bot_move_fun = BotHunt1;
}

void SB_PaintBattleBot(HDC hdc)
{
	SB_draw_field(hdc, indent, indent);
	SB_draw_field(hdc, field2leftindent, indent);
	SB_drawMisses(hdc, indent, indent, SB_cells[0]);
	SB_drawMisses(hdc, field2leftindent, indent, SB_cells[1]);
	SB_drawShipsBatlle(hdc, indent, indent, SB_ShipCells[0], false);
	SB_drawShipsBatlle(hdc, field2leftindent, indent, SB_ShipCells[1], true);
	if (!player_flag)
	{
		bot_move_fun();
		Sleep(400);
		InvalidateRect(0, 0, false);
	}
	else
	{
		//TextOutA(hdc, indent + field_size / 2 - 15, 0, "Ход", 3);
	}
	if (dead_ships_counter[0] == 10)
	{
		SB_drawShipsBatlle(hdc, field2leftindent, indent, SB_ShipCells[1], true);
		MessageBoxA(0, "Вы проиграли.", "BETTER LUCK NEXT TIME!", MB_OK);
		InvalidateRect(0, 0, true);
		SB_init();
	}
	if (dead_ships_counter[1] == 10)
	{
		MessageBoxA(0, "Вы выиграли!", "WINNER WINNER CHICKEN DINNER!", MB_OK);
		InvalidateRect(0, 0, true);
		SB_init();
	}
}

void SB_draw_field(HDC hdc, int left, int top)
{
	int right = left + field_size;
	int x = left + 10;
	Rectangle(hdc, left, top, right + 1, top + field_size + 1);
	for (char i = 'a'; i < 'k'; i++) // 10 iterations
	{
		TextOutA(hdc, x, top - 18, &i, 1);
		x += cell_size;
	}
	int num_left = left - 10;
	int x1 = left + cell_size, y1 = top + cell_size;
	int y = top + 6;
	for (char i = '1'; i < ':'; i++) // 9 iterations
	{
		MoveToEx(hdc, left, y1, 0);	//horizon
		LineTo(hdc, right, y1);
		MoveToEx(hdc, x1, top, 0);	//vetrical
		LineTo(hdc, x1, top + field_size);
		y1 += cell_size;
		x1 += cell_size;
		TextOutA(hdc, num_left, y, &i, 1);
		y += cell_size;
	}
	const char ten[2] = { '1', '0' };
	TextOutA(hdc, num_left - 10, y, ten, 2);
}

#include "stdafx.h"