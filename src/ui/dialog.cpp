//
// lindi
// Copyright (C) 2014 Mars J. Saxman
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "dialog.h"
#include "frame.h"
#include <assert.h>

UI::Dialog::Base::Base(std::string prompt):
	_prompt(prompt)
{
}

void UI::Dialog::Base::layout(int vpos, int hpos, int height, int width)
{
	int content_height = 1;
	int new_height = std::min(content_height, height / 2);
	int new_vpos = vpos + height - new_height;
	inherited::layout(new_vpos, hpos, new_height, width);
}

bool UI::Dialog::Base::process(UI::Frame &ctx, int ch)
{
	switch (ch) {
		case Control::Escape:	// escape key
		case Control::Close:	// control-W
			// the user no longer wants this action
			// this dialog has no further purpose
			ctx.show_result("Cancelled");
			return false;
		default: break;
	}
	return true;
}

void UI::Dialog::Base::paint_into(WINDOW *view, bool active)
{
	// Everything drawn in a dialog is reversed by default.
	wattron(view, A_REVERSE);
	// Fill the dialog window.
	wmove(view, 0, 0);
	int height, width;
	getmaxyx(view, height, width);
	(void)height; // unused
	whline(view, ' ', width);
	waddnstr(view, _prompt.c_str(), width);
	wattroff(view, A_REVERSE);
}

void UI::Dialog::Base::set_help(HelpBar::Panel &panel)
{
	panel.label[1][0] = HelpBar::Label('[', true, "Escape");
}

UI::Dialog::Input::Input(std::string prompt, action_t commit):
	Base(prompt),
	_commit(commit)
{
}

bool UI::Dialog::Input::process(UI::Frame &ctx, int ch)
{
	switch (ch) {
		case Control::Return:
		case Control::Enter:
			// the user is happy with their choice
			// tell the action to proceed and then
			// inform our host that we are finished
			if (_commit) _commit(ctx, _value);
			return false;
		case KEY_LEFT: arrow_left(); break;
		case KEY_RIGHT: arrow_right(); break;
		case Control::Backspace: delete_prev(); break;
		case KEY_DC: delete_next(); break;
		default:
			// we only care about non-control chars now
			if (ch < 32 || ch > 127) break;
			// in all other situations, the keypress should be
			// inserted into the field at the cursor point.
			key_insert(ch);
			break;
	}
	if (_repaint) {
		ctx.repaint();
	}
	return inherited::process(ctx, ch);
}

bool UI::Dialog::Input::poll(Frame &ctx)
{
	if (_repaint) {
		ctx.repaint();
	}
	return inherited::poll(ctx);
}

void UI::Dialog::Input::paint_into(WINDOW *view, bool active)
{
	inherited::paint_into(view, active);

	wattron(view, A_REVERSE);
	waddstr(view, ": ");
	int height, width;
	getmaxyx(view, height, width);
	(void)height;	//unused
	int value_vpos, value_hpos;
	getyx(view, value_vpos, value_hpos);
	(void)value_vpos; // unused
	waddnstr(view, _value.c_str(), width - value_hpos);
	int end_vpos, end_hpos;
	getyx(view, end_vpos, end_hpos);
	(void)end_vpos; // unused
	whline(view, ' ', width - end_hpos);

	// We're done being all reversed and stuff.
	wattroff(view, A_REVERSE);

	// Put the cursor where it ought to be. Make it visible, if that
	// would be appropriate for our activation state.
	wmove(view, 0, value_hpos + _cursor_pos);
	curs_set(active? 1: 0);
}

void UI::Dialog::Input::arrow_left()
{
	_cursor_pos -= std::min(_cursor_pos, 1U);
	_repaint = true;
}

void UI::Dialog::Input::arrow_right()
{
	_cursor_pos++;
	_repaint = true;
}

void UI::Dialog::Input::delete_prev()
{
	select_field();
	if (_value.empty()) return;
	if (_cursor_pos == 0) return;
	_cursor_pos--;
	auto deliter = _value.begin() + _cursor_pos;
	_value.erase(deliter);
	_repaint = true;
}

void UI::Dialog::Input::delete_next()
{
	select_field();
	if (_value.empty()) return;
	if (_cursor_pos >= _value.size()) return;
	auto deliter = _value.begin() + _cursor_pos;
	_value.erase(deliter);
	_repaint = true;
}

void UI::Dialog::Input::key_insert(int ch)
{
	select_field();
	_value.insert(_cursor_pos++, 1, ch);
	_repaint = true;
}

void UI::Dialog::Input::select_field()
{
	if (field_selected()) return;
	_cursor_pos = _value.size();
	_repaint = true;
}

void UI::Dialog::Input::set_value(std::string val)
{
	if (val == _value) return;
	_value = val;
	_repaint = true;
}

UI::Dialog::Confirmation::Confirmation(std::string p, action_t y, action_t n):
	Base(p),
	_yes(y),
	_no(n)
{
}

bool UI::Dialog::Confirmation::process(UI::Frame &ctx, int ch)
{
	switch (toupper(ch)) {
		case 'Y': _yes(ctx); return false;
		case 'N': _no(ctx); return false;
		case 'A': if (_all) { _all(ctx); return false; }
		default: break;
	}
	return inherited::process(ctx, ch);
}

void UI::Dialog::Confirmation::set_help(HelpBar::Panel &panel)
{
	inherited::set_help(panel);
	panel.label[0][0] = HelpBar::Label('Y', false, "Yes");
	panel.label[0][1] = HelpBar::Label('N', false, "No");
	if (_all) {
		panel.label[0][2] = HelpBar::Label('A', false, "All");
	}
}

