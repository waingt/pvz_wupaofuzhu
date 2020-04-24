#include "window.h"
#include "pvz.h"

extern window::Window* win;
namespace window {
	Window::Window() : Fl_Double_Window(330, 550, "无炮辅助")
	{
		wave = new Label<>(10, 10, 80, 30, "第？波");
		(new Fl_Check_Button(100, 10, 90, 30, "总在最前"))
			->callback([](Fl_Widget* w) {SetWindowPos(win->hwnd, (HWND)(((Fl_Check_Button*)w)->value() - 2), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); });
		(new Fl_Button(250, 10, 70, 30, "关于"))
			->callback([](Fl_Widget* w) {win->info("Get source code at https://github.com/waingt/pvz_wupaofuzhu"); });

		new Fl_Box(10, 50, 80, 30, "刷新倒计时");
		pgs_countdown = new ProgressBar<>(100, 50, 220, 30, "??");

		new Fl_Box(10, 90, 80, 30, "波长：");
		pgs_wavelength = new ProgressBar<>(100, 90, 220, 30, "??", 1);
		pgs_wavelength->color(FL_WHITE, FL_GREEN);

		(new Fl_Check_Button(10, 130, 90, 30, "自动收集"))
			->callback([](Fl_Widget* w) {pvz::auto_collect_on = ((Fl_Check_Button*)w)->value(); });
		(new Fl_Check_Button(100, 130, 110, 30, "樱桃秒炸开盒"))
			->callback([](Fl_Widget* w) {pvz::auto_use_cherry_on = ((Fl_Check_Button*)w)->value(); });
		btn_autofillice = new Fl_Check_Button(210, 130, 90, 30, "自动存冰");
		btn_autofillice->callback([](Fl_Widget* w) {pvz::auto_fill_ice_on = ((Fl_Check_Button*)w)->value(); });

		{auto t = new Fl_Input(70, 160, 250, 30, "存冰位");
		t->value(text_ice_storage);
		t->callback(pvz::cb_ice_storage);
		t->when(FL_WHEN_ENTER_KEY);
		input_ice_storage = t; }

		(new Fl_Check_Button(10, 200, 110, 30, "刷新时自动用冰"))
			->callback([](Fl_Widget* w) {pvz::auto_use_ice_on = ((Fl_Check_Button*)w)->value(); });
		new Fl_Box(120, 200, 70, 30, "冰总CD");
		pgs_icecd = new ProgressBar<>(190, 200, 130, 30, "??", 1);

		(new Fl_Check_Button(10, 230, 110, 30, "自动修复南瓜"))
			->callback([](Fl_Widget* w) {pvz::auto_fix_pumpkin = ((Fl_Check_Button*)w)->value(); });
		new Fl_Box(120, 230, 70, 30, "破损率：");
		pgs_broken_ratio = new ProgressBar<>(190, 230, 130, 30, "?%", 4000);

		new Fl_Box(10, 270, 70, 30, "修补阈值");
		{auto t = new Fl_Value_Slider(80, 270, 150, 30);
		t->type(5);
		t->maximum(4000);
		t->step(1);
		t->value(2000);
		pumpkin_threshold = t; }
		{auto t = new Fl_Check_Button(230, 270, 80, 30, "自动阈值");
		t->value(1);
		t->callback([](Fl_Widget* w) {pvz::auto_threshold == ((Fl_Check_Button*)w)->value(); }); }

		(new Fl_Check_Button(10, 310, 220, 30, "中键点击即发炮(双击对称发炮"))
			->callback(pvz::cb_change_hook);
		(new Fl_Check_Button(230, 310, 80, 30, "女仆秘籍"))
			->callback([](Fl_Widget* w) {pvz::maid_cheat_on = ((Fl_Check_Button*)w)->value(); });

		zombie_kind_list = new TextDisplay(10, 360, 140, 70, "刷怪类型");
		plants_without_fullhp = new TextDisplay(160, 360, 160, 70, "植物残血");

		console = new Console(10, 450, 310, 90, "Console");
		end();
		show();
		wait_for_expose();
		hwnd = fl_xid(this);
	}
	Console::Console(int X, int Y, int W, int H, const char* l) :Fl_Text_Display(X, Y, W, H, l)
	{
		buff = new Fl_Text_Buffer();
		buffer(buff);
		linenumber_format("%d");
	}
	/*template<>
	void Console::Writeline(const char* string)*/
}