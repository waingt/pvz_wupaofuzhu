#pragma once
#include <Windows.h>
#include <FL/Fl.H>

namespace pvz {
	extern char auto_collect_on, auto_use_cherry_on, auto_fill_ice_on, auto_use_ice_on, auto_fix_pumpkin, auto_threshold, maid_cheat_on;
	extern HWND hwnd;
	extern int game_clock, game_ui, game_scene, row_max, slots_num;
	extern float dpi_scale;
	extern char card2slot[96];

	void loop(void* data);
	void init();
	void finalize();
	void cb_ice_storage(Fl_Widget* w);
	void cb_change_hook(Fl_Widget* w);
}