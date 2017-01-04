
#include <gtkmm.h>
#include "OMainWnd.h"

int main(int argc, char *argv[]) {
	Gtk::Main main_obj(argc, argv);
	OMainWnd window_obj;
	main_obj.run(window_obj);
	return 0;
}
