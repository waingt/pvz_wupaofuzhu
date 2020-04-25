#include <Windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
	return TRUE;
}
template <class... cls>
void debug(LPCWSTR format, cls... args)
{
	WCHAR buff[100];
	wsprintf(buff, format, args...);
	MessageBoxW(0, buff, L"", MB_OK);
}
struct Point {
	int x, y;
};
void debug(Point pt)
{
	WCHAR buff[100];
	wsprintf(buff, L"%3d,%3d", pt.x, pt.y);
	MessageBoxW(0, buff, L"", MB_OK);
}
inline bool safe_distance(Point pt1, Point pt2) {
	int t1 = pt1.x - pt2.x, t2 = pt1.y - pt2.y;
	return t1 * t1 + t2 * t2 >= 10000;
}
inline bool find_proper_clkpoint(int game_scene, int row, int col, Point& clkpoint, Point fall) {
	bool backyard = game_scene == 2 || game_scene == 3;
	clkpoint = { 80 * col + 40,backyard ? 85 * row + 80 : 100 * row + 80 };
	if (safe_distance(clkpoint, fall))return true;
	clkpoint = { 80 * col + 199,backyard ? 85 * row + 164 : 100 * row + 179 };
	if (safe_distance(clkpoint, fall))return true;
	return false;
}
Point grid2xy(int row, int col, int game_scene) {
	row++; col++;
	int x, y;
	x = 80 * col;
	switch (game_scene)
	{
	case 2:
	case 3:
		y = 55 + 85 * row;
		break;
	case 4:
	case 5:
		if (col >= 6)
			y = 45 + 85 * row;
		else
			y = 45 + 85 * row + 20 * (6 - col);
		break;
	default:
		y = 40 + 100 * row;
		break;
	}
	return{ x,y };
}
HWND hwnd;
void click(Point pt) {
	auto lp = MAKELONG(pt.x, pt.y);
	PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lp);
	PostMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, lp);
}
void safe_click() {
	PostMessage(hwnd, WM_RBUTTONDOWN, MK_RBUTTON, 0);
	PostMessage(hwnd, WM_RBUTTONUP, MK_RBUTTON, 0);
}
Point getMousePoint(PMSG msg) {
	int t = msg->lParam;
	return { LOWORD(t),HIWORD(t) };
}
void doubleshoot(Point fall1) {
	if (fall1.y < 80 || fall1.y > 600)return;
	auto base = *(int*)0x6A9EC0;
	if (*(int*)(base + 0x7FC) != 3)return;
	auto game = *(int*)(base + 0x768);
	if (*(bool*)(game + 0x164)) return;
	Point fall2 = fall1;
	auto game_scene = *(int*)(game + 0x554C);
	if (game_scene == 2 || game_scene == 3) {
		fall2.y = fall1.y < 335 ? 462 : 207;
	}
	else {
		fall2.y += (fall1.y < 330 ? 200 : -200);
	}
	Point cannon[2], cannon_clk[2];
	int cannon_found = 0;
	auto p = *(int*)(game + 0xAC);
	auto max = *(int*)(game + 0xB0);
	for (int i = 0; i < max && cannon_found < 2; i++, p += 0x14C)
	{
		if (!*(short*)(p + 0x141) && *(int*)(p + 0x24) == 47 && *(int*)(p + 0x3C) == 37)
			cannon[cannon_found++] = { *(int*)(p + 0x1C), *(int*)(p + 0x28) };
	}
	if (cannon_found < 2)return;
	if (!(find_proper_clkpoint(game_scene, cannon[0].x, cannon[0].y, cannon_clk[0], fall1)
		&& find_proper_clkpoint(game_scene, cannon[1].x, cannon[1].y, cannon_clk[1], fall2))) {
		Point t = cannon[0];
		cannon[0] = cannon[1];
		cannon[1] = t;
		find_proper_clkpoint(game_scene, cannon[0].x, cannon[0].y, cannon_clk[0], fall1);
		find_proper_clkpoint(game_scene, cannon[1].x, cannon[1].y, cannon_clk[1], fall2);
	}
	safe_click();
	click(cannon_clk[0]);
	click(fall1);
	click(cannon_clk[1]);
	click(fall2);
	safe_click();
}

void shoot(Point fall) {
	if (fall.y < 80 || fall.y > 600)return;
	auto base = *(int*)0x6A9EC0;
	if (*(int*)(base + 0x7FC) != 3) return;
	auto game = *(int*)(base + 0x768);
	if (*(bool*)(game + 0x164)) return;
	auto p = *(int*)(game + 0xAC);
	auto max = *(int*)(game + 0xB0);
	for (int i = 0; i < max; i++, p += 0x14C)
	{
		if (!*(short*)(p + 0x141) && *(int*)(p + 0x24) == 47 && *(int*)(p + 0x3C) == 37)
		{
			auto game_scene = *(int*)(game + 0x554C);
			auto cannon_pt = grid2xy(*(int*)(p + 0x1C), *(int*)(p + 0x28), game_scene);
			if (safe_distance(cannon_pt, fall)) {
				safe_click();
				click(cannon_pt);
				click(fall);
				safe_click();
				break;
			}
		}
	}


}

extern "C" {
	__declspec(dllexport)LRESULT MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
	{

		if (nCode >= 0)
		{
			auto msg = (PMSG)lParam;
			Point mouse;
			hwnd = msg->hwnd;
			switch (msg->message)
			{
			case WM_MBUTTONDOWN:
				msg->message = WM_NULL;
				mouse = getMousePoint(msg);
				//debug(L"%d,%d", mouse.x, mouse.y);
				shoot(mouse);
				break;
			case WM_RBUTTONDBLCLK:
				msg->message = WM_NULL;
				mouse = getMousePoint(msg);
				doubleshoot(mouse);
				break;
			}
		}
		return CallNextHookEx(0, nCode, wParam, lParam);
	}
}