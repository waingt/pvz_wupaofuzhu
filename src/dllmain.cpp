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
	int square_dist(int x1, int y1) {
		int t1 = x - x1, t2 = y - y1;
		return t1 * t1 + t2 * t2;
	}
};
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
Point getMousePoint(PMSG msg) {
	int t = msg->lParam;
	return { LOWORD(t),HIWORD(t) };
}

void shoot(Point mouse, HWND hwnd) {
	if (mouse.y < 90)return;
	auto base = *(int*)0x6A9EC0;
	if (*(int*)(base + 0x7FC) == 3) {
		auto game = *(int*)(base + 0x768);
		if (*(bool*)(game + 0x164) == 0) {
			auto p = *(int*)(game + 0xAC);
			auto max = *(int*)(game + 0xB0);
			for (int i = 0; i < max; i++, p += 0x14C)
			{
				if (!*(short*)(p + 0x141) && *(int*)(p + 0x24) == 47 && *(int*)(p + 0x3C) == 37)
				{
					auto game_scene = *(int*)(game + 0x554C);
					auto cannon_pt = grid2xy(*(int*)(p + 0x1C), *(int*)(p + 0x28), game_scene);
					auto t = cannon_pt.square_dist(mouse.x, mouse.y);
					//debug(L"%d", t);
					if (t < 10000) {
						continue;
					}

					PostMessage(hwnd, WM_RBUTTONDOWN, MK_RBUTTON, 0);
					PostMessage(hwnd, WM_RBUTTONUP, MK_RBUTTON, 0);

					auto lp = MAKELONG(cannon_pt.x, cannon_pt.y);
					PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lp);
					PostMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, lp);

					lp = MAKELONG(mouse.x, mouse.y);
					PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lp);
					PostMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, lp);

					break;
				}
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
			HWND hwnd = msg->hwnd;
			switch (msg->message)
			{
			case WM_MBUTTONDOWN:
				msg->message = WM_NULL;
				mouse = getMousePoint(msg);
				//debug(L"%d,%d", mouse.x, mouse.y);
				shoot(mouse, hwnd);
				break;
			case WM_MBUTTONDBLCLK:
				msg->message = WM_NULL;
				mouse = getMousePoint(msg);
				mouse.y = 690 - mouse.y;
				shoot(mouse, hwnd);
				break;
			case WM_MBUTTONUP:
				msg->message = WM_NULL;
				break;
			}
		}
		return CallNextHookEx(0, nCode, wParam, lParam);
	}
}