#include "pvz.h"
#include "window.h"
#include "pvz_class.h"
#include "operation.h"
#include "utils.h"
#include <signal.h>
#include <vector>
#include <cstring>
#include <cmath>
#include <algorithm>

extern window::Window* win;
namespace pvz {
	/*
	TODO: refine the value of can_plant: considering craters and ice trails
	*/
	/*
	Stateful variables are:
	auto_fill_ice_on
	maid_cheat_on
	dancer_clock
	game_clock
	base
	last_refresh_clock
	last_wave
	*/
	float dpi_scale;
	HWND hwnd;
	HANDLE hProcess;
	HMODULE hModule;
	HOOKPROC hook;
	DWORD ThreadId;
	HHOOK hhook;
	char auto_collect_on, auto_use_cherry_on, auto_fill_ice_on, auto_use_ice_on, auto_fix_pumpkin, auto_threshold = 1, maid_cheat_on;
	bool dancer_clock, zombie_kind[33];;
	char card2slot[96];
	int game_clock, game_ui, game_scene, row_max, slots_num;
	int base, game, zombies_offset, plants_offset, slots_offset;
	int cherry_cd_offset, ice_cd_offset, imitated_ice_cd_offset, coffee_cd_offset, pumpkin_cd_offset, imitated_pumpkin_cd_offset;
	info zombies_info, plants_info, items_info;
	plant plants[500];
	zombie zombies[500];
	item items[500];
	int wave, countdown, last_refresh_clock, last_wave, total_icecd;
	char can_plant[6][9];//0=nothing,1=lilipad,2=water,3=has_plant
	struct plant_simple {
		int hp, row, col, kind;
		inline bool operator <(plant_simple& p) {
			return hp < p.hp;
		}
	};
	plant_simple plants_without_fullhp[200], pumpkins[200];
	std::vector<std::pair<int, int>> icestorage;
	const char* card2name[] = { "豌豆", "向日葵", "樱桃", "坚果", "土豆雷", "寒冰射手", "大嘴花", "双发", "小喷菇", "阳光菇", "大喷菇", "墓碑", "魅惑菇", "胆小菇", "寒冰菇", "毁灭菇", "睡莲", "窝瓜", "三线射手", "缠绕海草", "辣椒", "地刺", "火炬树桩", "高坚果", "海蘑菇", "灯笼", "仙人掌", "三叶草", "裂荚", "杨桃", "南瓜", "磁力菇", "卷心 菜", "花盆", "玉米", "咖啡豆", "大蒜", "保护伞", "金盏花", "西瓜", "机枪", "双子向日葵", "忧郁菇", "香蒲", "冰瓜", "吸金磁", "地刺王", "玉米加农炮", "复制 豌豆", "复制向日葵", "复制樱桃", "复制坚果", "复制土豆雷", "复制寒冰射手", "复制大嘴花", "复制双发", "复制小喷菇", "复制阳光菇", "复制大喷菇", "复制墓碑", "复制魅惑菇", "复制胆小菇", "复制寒冰菇", "复制毁灭菇", "复制睡莲", "复制窝瓜", "复制三线射手", "复制缠绕海草", "复制辣椒", "复制地刺", "复制火炬树桩", "复制高坚果", "复制海蘑菇", "复制灯笼", "复制仙人掌", "复制三叶草", "复制裂荚", "复制杨桃", "复制南瓜", "复制磁力菇", "复制卷心菜", "复制花盆", "复制玉米", " 复制咖啡豆", "复制大蒜", "复制保护伞", "复制金盏花", "复制西瓜", "复制机枪", "复制双子向日葵", "复制忧郁菇", "复制香蒲", "复制冰瓜", "复制吸金磁", "复制地 刺王", "复制玉米加农炮" };

	void unhook();

	inline int read_int(int addr) {
		int buff;
		ReadProcessMemory(hProcess, (LPCVOID)addr, &buff, sizeof(buff), NULL);
		return buff;
	}
	template<class ...T>
	inline int read_int(int first, int second, T... args) {
		return read_int(read_int(first) + second, args...);
	}
	template<typename T>
	inline bool read(int addr, T& buff) {
		return ReadProcessMemory(hProcess, (LPCVOID)addr, &buff, sizeof(T), NULL);
	}
	template<typename T>
	inline bool read(int addr, T buff[], int count) {
		return ReadProcessMemory(hProcess, (LPCVOID)addr, buff, sizeof(T) * count, NULL);
	}
	template<int expected_maximum>
	inline float linear_smooth(float x) {
		return x / expected_maximum;
	}
	template<int expected_maximum>
	inline float log_smooth(float x) {
		constexpr auto ratio = ((float)expected_maximum) / 20;
		return logf(x / ratio + 1) / 3;
	}
	template<int expected_maximum>
	inline float sqrt_smooth(float x) {
		constexpr auto ratio = ((float)expected_maximum) / 15;
		return (sqrtf(x / ratio + 1) - 1) / 3;
	}
	bool check_process() {
		if (hProcess) {
			DWORD exitcode;
			GetExitCodeProcess(hProcess, &exitcode);
			if (exitcode != STILL_ACTIVE) {
				win->debug("pvz closed");
				unhook();
				CloseHandle(hProcess);
				hProcess = 0;
				ThreadId = 0;
				base = 0;
			}
		}
		else
		{
			hwnd = FindWindow(TEXT("MainWindow"), TEXT("Plants vs. Zombies"));
			if (hwnd == NULL)return false;
			DWORD pid;
			ThreadId = GetWindowThreadProcessId(hwnd, &pid);
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
			if (!hProcess) win->info("cannot open pvz process!!");
			else win->info("pvz opened");
			read(0x6A9EC0, base);
		}
		return hProcess;
	}
	void auto_use_cherry();
	void auto_collect();
	void auto_fill_ice();
	void loop(void* data) {
		if (!check_process())return;
		read(base + 0x7FC, game_ui);
		if (game_ui == 3)
		{
			if (!game) {
				// initialize when entering combat
#pragma region
				game_clock = -1;
				slot slots[10];
				win->debug("pvz enter combat");
				read(base + 0x768, game);
				read(game + 0x554C, game_scene);
				row_max = (game_scene == 2 || game_scene == 3) ? 5 : 4;
				read(game + 0x556C, game_clock);
				last_refresh_clock = countdown = game_clock;
				read(game + 0x557C, wave);
				last_wave = wave;
				read(game + 0x144, slots_offset);
				read(slots_offset + 0x24, slots_num);
				read(slots_offset + 0x28, slots, slots_num);
				memset(card2slot, -1, sizeof(card2slot));
				for (int i = 0; i < slots_num; i++)
				{
					int kind = slots[i].kind == 48 ? slots[i].imitater_kind + 48 : slots[i].kind;
					card2slot[kind] = i;
				}
				cherry_cd_offset = card2slot[2] >= 0 ? slots_offset + 0x4C + 0x50 * card2slot[2] : 0;
				ice_cd_offset = card2slot[14] >= 0 ? slots_offset + 0x4C + 0x50 * card2slot[14] : 0;
				imitated_ice_cd_offset = card2slot[62] >= 0 ? slots_offset + 0x4C + 0x50 * card2slot[62] : 0;
				coffee_cd_offset = card2slot[35] >= 0 ? slots_offset + 0x4C + 0x50 * card2slot[35] : 0;
				pumpkin_cd_offset = card2slot[30] >= 0 ? slots_offset + 0x4C + 0x50 * card2slot[30] : 0;
				imitated_pumpkin_cd_offset = card2slot[78] >= 0 ? slots_offset + 0x4C + 0x50 * card2slot[78] : 0;
				read(game + 0x54D4, zombie_kind);
				char buff[1000], * p = buff, count = 0;
				const char zombies_cared[] = { 32, 23, 22, 21, 20, 18, 17, 16, 15, 14, 12, 8, 7, 3 };
				const char* zombies_cared_name[] = { "红眼", "白眼", "篮球", "梯子", "蹦极", "跳跳", "矿工", "气球", "小丑", "海豚", "冰车", "MJ", "橄榄", "撑杆" };
				for (int i = 0; i < sizeof(zombies_cared); i++)
				{
					if (zombie_kind[zombies_cared[i]]) {
						for (const char* src = zombies_cared_name[i]; *src; src++, p++)*p = *src;
						count++;
						*p++ = count % 3 ? ' ' : '\n';
					}
				}
				*p = '\0';
				win->zombie_kind_list->text(buff);
#pragma endregion
			}
			else {
				if (maid_cheat_on) {
					bool should_pause = (read_int(base + 0x838) + 10) % 460 >= 240;
					if (maid_cheat_on == 1)maid_cheat_on = 2;
					else if (should_pause != dancer_clock)press_space();
					dancer_clock = should_pause;
				}
				int t;
				read(game + 0x556C, t);
				if (t == game_clock)return;
				else game_clock = t;
				read(game + 0x557C, wave);
				read(game + 0x559C, countdown);

			}
			// show wave number and countdown
#pragma region
			sprintf_s(win->wave->text, "第%d波", wave);
			win->wave->redraw_label();
			if (wave == 9 || wave == 19)
			{
				win->pgs_countdown->maximum(1200);
				win->pgs_countdown->color(FL_WHITE, countdown > 200 ? FL_GREEN : (countdown < 195 ? FL_YELLOW : FL_RED));
				if (countdown > 5) {
					countdown += 745;
				}
				else
				{
					int big_wave_countdown;
					read(game + 0x55A4, big_wave_countdown);
					countdown = big_wave_countdown;
				}
			}
			else if (wave == 20)
			{
				win->pgs_countdown->maximum(5500);
				win->pgs_countdown->color(FL_WHITE, FL_GREEN);
			}
			else
			{
				win->pgs_countdown->maximum(300);
				win->pgs_countdown->color(FL_WHITE, countdown > 200 ? FL_GREEN : (countdown < 195 ? FL_YELLOW : FL_RED));
			}
			win->pgs_countdown->value(countdown);
			_itoa_s(countdown, win->pgs_countdown->text, 10);
#pragma endregion

			// plants loop begin
#pragma region
			read(game + 0xAC, plants_info);
			read(plants_info.offset, plants, plants_info.max_num);
			total_icecd = 0;
			int l1 = 0, l2 = 0, pumpkin_total_hp = 0, ice_row = -1, ice_col = -1;
			memset(can_plant, 0, sizeof(can_plant));
			if (row_max == 5)
				memset(((char*)can_plant) + 18, 2, 18);
			for (int i = 0, count = 0; i < plants_info.max_num && count < plants_info.current_num; i++)
			{
				auto& p = plants[i];
				if (p.disapeared_or_crushed)continue;
				count++;
				if (p.kind == 14) { total_icecd += 5000; ice_row = p.row, ice_col = p.col; }
				if (p.kind == 48 && *((char*)&p + 312) == 14)total_icecd += 5000;//模仿冰模仿中
				if (p.kind != 30 && p.kind != 33)
				{
					char& t = can_plant[p.row][p.col];
					if (p.kind != 16) {
						t = 3;
						if (p.kind == 47)can_plant[p.row][p.col + 1] = 3;
					}
					else if (t != 3)t = 1;
				}
				if (p.kind == 30)
				{
					pumpkins[l1++] = { p.hp,p.row,p.col,p.kind };
					pumpkin_total_hp += p.hp;
				}
				if (p.hp < p.max_hp && p.max_hp == 300)
				{
					plants_without_fullhp[l2++] = { p.hp,p.row,p.col,p.kind };
				}
			}
#pragma endregion

			// show ice total cd
#pragma region
			{int t = 0, num = 0, stuck = 0;
			if (ice_cd_offset) { read(ice_cd_offset, t); num++; }
			total_icecd += t;
			if (!t) { total_icecd += 5000; stuck++; }
			if (imitated_ice_cd_offset) { read(imitated_ice_cd_offset, t); num++; }
			total_icecd += t;
			if (!t) { total_icecd += 5000; stuck++; }
			_itoa_s(total_icecd, win->pgs_icecd->text, 10);
			win->pgs_icecd->value(sqrt_smooth<40000>(total_icecd));
			win->pgs_icecd->color(FL_WHITE, stuck == num ? FL_RED : (stuck ? FL_YELLOW : FL_BLUE)); }
#pragma endregion

			// auto fix pumpkin
#pragma region
			if (l1 > 0) {
				int average = pumpkin_total_hp / l1, thershold;
				win->pgs_broken_ratio->value(average);
				win->pgs_broken_ratio->color(FL_WHITE, average < 1333 ? FL_RED : (average < 2666 ? FL_YELLOW : FL_GREEN));
				_itoa_s(average / 40, win->pgs_broken_ratio->text, 10);
				{char* p = win->pgs_broken_ratio->text;
				for (; *p; p++);
				*p = '%'; *++p = '\0'; }
				if (auto_threshold) {
					thershold = 2666 - average / 3;
					win->pumpkin_threshold->value(thershold);
				}
				else thershold = win->pumpkin_threshold->value();
				if (thershold > 2666)thershold = 2666;
				auto& p = *std::min_element(pumpkins, pumpkins + l1);
				if (auto_fix_pumpkin && p.hp < thershold) {
					if (pumpkin_cd_offset && read_int(pumpkin_cd_offset) == 0)
						use_card(30, p.row, p.col);
					else if (imitated_pumpkin_cd_offset && read_int(imitated_pumpkin_cd_offset) == 0)
						use_card(78, p.row, p.col);
				}
			}
#pragma endregion

			// show plants without full hp
#pragma region
			std::sort(plants_without_fullhp, plants_without_fullhp + l2);
			char buff[1000], * s = buff;
			for (int i = 0; i < l2; i++)
			{
				auto& p = plants_without_fullhp[i];
				*s++ = ((char)p.row) + '1'; *s++ = ','; *s++ = ((char)p.col) + '1'; *s++ = ' ';
				for (const char* src = card2name[p.kind]; *src; s++, src++)*s = *src;
				*s++ = ':';
				_itoa_s(p.hp, s, buff + sizeof(buff) - s, 10); while (*s)s++;
				*s++ = '\n';
			}
			*s++ = '\0';
			win->plants_without_fullhp->text(buff);
#pragma endregion

			if (wave != last_wave)
			{
				last_wave = wave;
				last_refresh_clock = game_clock;
				// auto use ice
				if (auto_use_ice_on && ice_row >= 0 && wave != 10 && wave != 20) {
					if (coffee_cd_offset && read_int(coffee_cd_offset) == 0)use_card(35, ice_row, ice_col);
					else win->info("刷新时无法使用咖啡豆！");
				}
			}
			//show wave length
			float wave_length = game_clock - last_refresh_clock;
			win->pgs_wavelength->value(log_smooth<2000>(wave_length));
			auto t = win->pgs_wavelength->minimum();
			_itoa_s(wave_length, win->pgs_wavelength->text, 10);

			// auto fill ice
			if (auto_fill_ice_on)
				auto_fill_ice();

			// auto collect
			if (auto_collect_on && read_int(game + 0x138, 0x30) == 0)
				auto_collect();

			// auto use cherry
			if (auto_use_cherry_on && zombie_kind[15] && cherry_cd_offset && read_int(cherry_cd_offset) == 0)
				auto_use_cherry();
		}
		else
		{
			game = 0;
			game_clock = 0;
		}
	}
	void auto_collect() {
		read(game + 0xE4, items_info);
		read(items_info.offset, items, items_info.max_num);
		for (int i = 0; i < items_info.max_num; i++)
		{
			if (!items[i].disapeared && !items[i].collected && items[i].x >= 0 && items[i].y >= 70) {
				click(items[i].x + 30, items[i].y + 30);
				break;
			}
		}
	}
	void auto_use_cherry() {
		read(game + 0x90, zombies_info);
		read(zombies_info.offset, zombies, zombies_info.max_num);
		for (int i = 0; i < zombies_info.max_num; i++)
		{
			auto& zb = zombies[i];
			if (zb.disappeared)continue;
			if (zb.kind == 15 && zb.state == 16)
			{
				int row = zb.row, col = (zb.x - 10) / 80;
				int row_low = inrange(row - 1, 0, row_max), row_high = inrange(row + 1, 0, row_max);
				int col_low = inrange(col - 1, 0, 8), col_high = inrange(col + 1, 0, 8);
				bool is_threatening = false, place_found = false;
				int place_row, place_col;
				for (int i = row_low; i <= row_high; i++)
				{
					for (int j = col_low; j <= col_high; j++)
					{
						if (!is_threatening && can_plant[i][j] == 3)is_threatening = true;
						else if (!place_found && can_plant[i][j] <= 1) {
							place_row = i; place_col = j;
							place_found = true;
						}
						if (is_threatening && place_found)
						{
							use_card(2, place_row, place_col);
							win->info("樱桃炸丑于：(", place_row + 1, ',', place_col + 1, ") 爆炸倒计时：", zb.countdown);
							return;
						}
					}
				}
				if (is_threatening) {
					win->info("小丑爆炸但无法种植樱桃！(", row + 1, ',', col + 1, ") 爆炸倒计时：", zb.countdown);
					break;
				}
				else
				{
					win->debug("小丑爆炸但无威胁 (", row + 1, ',', col + 1, ") 爆炸倒计时：", zb.countdown);
					break;
				}
			}
		}
	}
	void auto_fill_ice() {
		if (auto_fill_ice_on == 1) {
			icestorage.clear();
			for (int i = 0, count = 0; i < plants_info.max_num && count < plants_info.current_num; i++)
				if (!plants[i].disapeared_or_crushed && plants[i].kind == 14)icestorage.push_back({ plants[i].row,plants[i].col });
			std::sort(icestorage.begin(), icestorage.end());
			auto& s = win->text_ice_storage;
			size_t i;
			for (i = 0; i < icestorage.size(); i++)
			{
				s[3 * i] = icestorage[i].first + '1';
				s[3 * i + 1] = icestorage[i].second + '1';
				s[3 * i + 2] = ',';
			}
			int len = max(3 * i - 1, 0);
			s[len] = '\0';
			win->input_ice_storage->static_value(s);
			auto_fill_ice_on = 2;
		}
		int row = -1, col = -1;
		for (auto& i : icestorage)
		{
			if (can_plant[i.first][i.second] <= 1) {
				row = i.first; col = i.second; break;
			}
		}
		if (row >= 0) {
			if (ice_cd_offset && read_int(ice_cd_offset) == 0)use_card(14, row, col);
			else if (imitated_ice_cd_offset && read_int(imitated_ice_cd_offset) == 0)use_card(62, row, col);
		}
	}
	void cb_ice_storage(Fl_Widget* w) {
		auto s = win->input_ice_storage->value();
		int len = strlen(s);
		if (len == 0)auto_fill_ice_on = 0;
		if (len % 3 != 2) { win->info("存冰位格式错误（正确格式为：行列[,行列...]）"); return; }
		std::vector<std::pair<int, int>> t;
		for (int i = 0; i < (len + 1) / 3; i++)
		{
			int row = s[3 * i] - '1';
			int col = s[3 * i + 1] - '1';
			if (col < 0 || col > 8 || row < 0 || row > row_max)
			{
				win->info("存冰位行列范围错误"); return;
			}
			if (s[3 * i + 2] != ',' && s[3 * i + 2] != '\0') { win->info("存冰位格式错误（正确格式为：行列[,行列...]）"); return; }
			t.push_back({ row,col });
		}
		icestorage = t;
		win->btn_autofillice->value(1);
		auto_fill_ice_on = 2;
		win->info("存冰位更新成功！");
	}
	void unhook() {
		if (hhook) {
			UnhookWindowsHookEx(hhook);
			hhook = 0;
			win->info("已卸载钩子");
		}
	}
	void cb_change_hook(Fl_Widget* w) {
		if (!ThreadId || !hModule)return;
		unhook();
		bool checked = ((Fl_Check_Button*)w)->value();
		if (checked) {
			hhook = SetWindowsHookEx(WH_GETMESSAGE, hook, hModule, ThreadId);
			if (hhook)win->info("消息钩子成功注入");
			else win->info("无法设置消息钩子");
		}
	}
	void init() {
		auto screen = GetDC(NULL);
		if (screen) {
			auto virtual_width = GetDeviceCaps(screen, HORZRES);
			auto physical_width = GetDeviceCaps(screen, DESKTOPHORZRES);
			ReleaseDC(NULL, screen);
			dpi_scale = (float)physical_width / virtual_width;
		}
		else {
			dpi_scale = 1.0;
		}
		hModule = LoadLibrary(L"ClickToShoot.dll");
		if (!hModule)win->info("找不到ClickToShoot.dll，无法使用点击即发炮");
		hook = (HOOKPROC)GetProcAddress(hModule, "MouseProc");
	}
	void finalize() {
		if (hhook) UnhookWindowsHookEx(hhook);
		if (hProcess) CloseHandle(hProcess);
		if (hModule)FreeLibrary(hModule);
	}
}