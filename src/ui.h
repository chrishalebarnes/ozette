#ifndef UI_H
#define UI_H

#include "window.h"
#include <vector>

class UI
{
public:
	class Delegate
	{
	public:
		virtual ~Delegate() = default;
		virtual void window_closed(std::unique_ptr<Window> &&win) = 0;
	};
	UI(Delegate &host);
	~UI();
	bool process(int ch, App &app);
	Window *open_window(std::unique_ptr<Controller> &&wincontrol);
	void make_active(Window *window);

	static int color_contrast() { return COLOR_PAIR(0); }
	static int color_highlight() { return COLOR_PAIR(s_color ? 1 : 0); }
	static int color_lowlight() { return COLOR_PAIR(s_color ? 2 : 0); }
	static int color_subdued() { return COLOR_PAIR(s_color ? 3 : 0); }
protected:
	// get the terminal width and height, then calculate column width
	void get_screen_size();
	// change the focus to a specific window
	void set_focus(size_t index);
	// reposition all the windows after create/remove/resize
	void relayout();
	// send this char to the focus window
	void send_to_focus(int ch, App &app);
	// close the window with this index & tell our delegate
	void close_window(size_t index);
	// run this once during setup: initialize the color system
	void init_colors();
	static bool s_color;
private:
	Delegate &_host;
	int _width = 0;
	int _height = 0;
	std::vector<std::unique_ptr<Window>> _columns;
	int _spacing = 0;
	int _columnWidth = 0;
	size_t _focus = 0;
};

#endif // UI_H
