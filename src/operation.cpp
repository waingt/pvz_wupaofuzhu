#include "operation.h"
#include "pvz.h"

namespace pvz {
	void click(int x, int y)
	{
		x /= dpi_scale;
		y /= dpi_scale;
		LPARAM position = ((y & 0xFFFF) << 16) | (x & 0xFFFF);
		PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, position);
		PostMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, position);
	}
	void safe_click()
	{
		PostMessage(hwnd, WM_RBUTTONDOWN, MK_RBUTTON, 0);
		PostMessage(hwnd, WM_RBUTTONUP, MK_RBUTTON, 0);
	}
	void press_space()
	{
		PostMessage(hwnd, WM_KEYDOWN, VK_SPACE, 0);
		PostMessage(hwnd, WM_KEYUP, VK_SPACE, 0);
	}
	void click_slot(int index) {
		index++;
		int x, y;
		switch (slots_num)
		{
		case 10:
			x = 63 + 51 * index;
			break;
		case 9:
			x = 63 + 52 * index;
			break;
		case 8:
			x = 61 + 54 * index;
			break;
		case  7:
			x = 61 + 59 * index;
			break;
		default:
			x = 61 + 59 * index;
			break;
		}
		y = 12;
		click(x, y);
	}
	void click_grid(int row, int col) {
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
		click(x, y);
	}
	bool inline check_in_scece(int row, int col) {
		return row <= row_max && row >= 0 && col >= 0 && col <= 8;
	}
	bool use_card(int card_index, int row, int col) {
		int slot_index = card2slot[card_index];
		if (slot_index < 0)return false;
		if (!check_in_scece(row, col)) return false;
		safe_click();
		click_slot(slot_index);
		click_grid(row, col);
		return true;
	}
}