#pragma once
#include <FL/x.H>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Value_Slider.H>
#include <sstream>
namespace window {
	template<int labelbuffsize = 10>
	class Label :public Fl_Box {
	public:
		char text[labelbuffsize];
		Label(int x, int y, int w, int h, const char* l) :Fl_Box(x, y, w, h) {
			strcpy_s(text, l);
			label(text);
		}
	};

	template<int labelbuffsize = 10>
	class ProgressBar :public Fl_Progress {
	public:
		char text[labelbuffsize];
		ProgressBar(int x, int y, int w, int h, const char* l) :Fl_Progress(x, y, w, h) {
			strcpy_s(text, l);
			label(text);
		}
		ProgressBar(int x, int y, int w, int h, const char* l, int max) :ProgressBar(x, y, w, h, l) {
			maximum(max);
		}
	};

	class TextDisplay :public Fl_Text_Display {
	public:
		Fl_Text_Buffer* textbuffer;
		TextDisplay(int x, int y, int w, int h, const char* l) :Fl_Text_Display(x, y, w, h, l) {
			textbuffer = new Fl_Text_Buffer();
			buffer(textbuffer);
		}
		inline void text(const char* s) { textbuffer->text(s); }
	};
	class Console :public Fl_Text_Display {
	public:
		Console(int X, int Y, int W, int H, const char* l);

		template<class ...Args>
		void Writeline(Args... objects)
		{
			std::stringstream ss;
			(ss << ... << objects) << '\n';
			auto s = ss.str();
			if (buff->length() > 1000)
				buff->remove(0, s.length());
			buff->append(s.c_str());
			scroll(mNBufferLines + 3 - mNVisibleLines, mHorizOffsetHint);
		}
	private:
		Fl_Text_Buffer* buff;
	};
	class Window :public Fl_Double_Window {
	public:
		ProgressBar<>* pgs_icecd, * pgs_countdown, * pgs_wavelength, * pgs_broken_ratio;
		Fl_Check_Button* btn_autofillice;
		Fl_Input* input_ice_storage;
		TextDisplay* zombie_kind_list, * plants_without_fullhp;
		Fl_Value_Slider* pumpkin_threshold;
		Label<>* wave;
		char text_ice_storage[100];
		Console* console;
		HWND hwnd;

		Window();

		template<class ...Args>
		inline void debug(Args ...objects)
		{
#ifdef _DEBUG
			console->Writeline(objects...);
#endif
		}
		template<class ...Args>
		inline void info(Args ...objects)
		{
			console->Writeline(objects...);
		}

	};
}