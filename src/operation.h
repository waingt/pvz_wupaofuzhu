#include <Windows.h>
#include <unordered_map>

namespace pvz {
	extern const char* card2name[96];


	void click(int x, int y);
	void safe_click();
	void press_space();
	void click_grid(int row, int col);
	void click_slot(int index);
	bool use_card(int card_index, int row, int col);
}