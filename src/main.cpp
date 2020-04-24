#include "window.h"
#include "pvz.h"
#include "pvz_class.h"
#include <iostream>

window::Window* win;
int main() {
	win = new window::Window();
	pvz::init();
	while (Fl::first_window())
	{
		Fl::wait(0.001);
		pvz::loop(0);
	}
	delete win;
	pvz::finalize();
	return 0;
}