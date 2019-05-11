/*
  Copyright 2017 Detlef Urban <onkel@paraair.de>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <math.h>
#include "ODial.h"
#include <gdkmm/general.h>  // for cairo helper functions
#include <iostream>
#include <cstring>

#define deg2rad(x) x*(3.14159265/180)

ODial::ODial() :
Glib::ObjectBase("ODial"),
m_label(0),
m_map(0),
m_value_callback(0),
Gtk::Widget() {
	set_has_window(true);

	m_value = 0;
	m_in_motion = false;
	m_min = 0;
	m_default = 0;
	m_max = 1;
	m_scroll_step = 5;

	set_size_request(40, 54);
	set_knob_background_color(0.6, 0.6, 0.6, 1);
	set_halign(Gtk::ALIGN_CENTER);
	set_hexpand(false);
	set_name("o-dial");


}

ODial::~ODial() {
	if (m_label)
		free(m_label);
}

void ODial::set_view_type(VIEW_TYPE view_type) {
	if (view_type == SINGLE_DSP)
		set_size_request(80, 36);
	if (view_type == NORMAL)
		set_size_request(40, 54);

	m_view_type = view_type;
}

Gtk::SizeRequestMode ODial::get_request_mode_vfunc() const {
	//Accept the default value supplied by the base class.
	return Gtk::Widget::get_request_mode_vfunc();
}

void ODial::get_preferred_width_vfunc(int& minimum_width, int& natural_width) const {
	minimum_width = 30;
	natural_width = 30;
}

void ODial::get_preferred_height_for_width_vfunc(int /* width */,
		int& minimum_height, int& natural_height) const {
	minimum_height = 30;
	natural_height = 30;
}

void ODial::get_preferred_height_vfunc(int& minimum_height, int& natural_height) const {
	minimum_height = 30;
	natural_height = 30;
}

void ODial::get_preferred_width_for_height_vfunc(int /* height */,
		int& minimum_width, int& natural_width) const {
	minimum_width = 30;
	natural_width = 30;
}

void ODial::on_size_allocate(Gtk::Allocation& allocation) {
	set_allocation(allocation);

	if (m_refGdkWindow) {
		m_refGdkWindow->move_resize(allocation.get_x(), allocation.get_y(),
				allocation.get_width(), allocation.get_height());
	}
}

void ODial::on_map() {
	Gtk::Widget::on_map();
}

void ODial::on_unmap() {
	Gtk::Widget::on_unmap();
}

void ODial::on_realize() {
	set_realized();

	if (!m_refGdkWindow) {
		//Create the GdkWindow:

		GdkWindowAttr attributes;
		memset(&attributes, 0, sizeof (attributes));

		Gtk::Allocation allocation = get_allocation();

		//Set initial position and size of the Gdk::Window:
		attributes.x = allocation.get_x();
		attributes.y = allocation.get_y();
		attributes.width = allocation.get_width();
		attributes.height = allocation.get_height();

		attributes.event_mask = get_events() |
				GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK |
				GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
				GDK_POINTER_MOTION_HINT_MASK | GDK_SCROLL_MASK;
		attributes.window_type = GDK_WINDOW_CHILD;
		attributes.wclass = GDK_INPUT_OUTPUT;

		m_refGdkWindow = Gdk::Window::create(get_parent_window(), &attributes,
				GDK_WA_X | GDK_WA_Y);
		set_window(m_refGdkWindow);

		m_refGdkWindow->set_user_data(gobj());
	}
}

void ODial::on_unrealize() {
	m_refGdkWindow.reset();

	Gtk::Widget::on_unrealize();
}

bool ODial::on_button_press_event(GdkEventButton* event) {
	if (event->button == 1) {
		m_in_motion = true;
		m_last_y = abs(event->y);

		return true;
	}
	if (event->button == 3) {
		set_value(m_default);

		return true;
	}

	return false;
}

bool ODial::on_button_release_event(GdkEventButton* event) {
	if (event->button == 1) {
		m_in_motion = false;

		return true;
	}

	return false;
}

bool ODial::on_motion_notify_event(GdkEventMotion* event) {
	if (m_refGdkWindow && m_in_motion) {
		m_value += (m_last_y - (int) (event->y));
		m_last_y = (int) (event->y);
		if (m_value < m_min)
			m_value = m_min;
		if (m_value > m_max)
			m_value = m_max;
		set_value(m_value);

		return true;
	}

	return false;
}

bool ODial::on_scroll_event(GdkEventScroll* event) {
	m_value += (event->direction == GDK_SCROLL_UP ? m_scroll_step : -m_scroll_step);
	if (m_value < m_min)
		m_value = m_min;
	if (m_value > m_max)
		m_value = m_max;
	set_value(m_value);

	return true;
}

bool ODial::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
	const Gtk::Allocation allocation = get_allocation();
	auto refStyleContext = get_style_context();

	gint i;
	gint width = m_view_type == SINGLE_DSP ? 40 : allocation.get_width();
	int height = allocation.get_height();

	gint center_x = width / 2;
	gint center_y = m_view_type == SINGLE_DSP ? 20 : (height) / 2;

	gint outer_radius = width / 2 - 4; // 4 is padding
	gint tick_length = 4;
	gint inner_radius = outer_radius - tick_length;

	// draw knob title		
	if (m_label) {
		if (m_view_type == SINGLE_DSP)
			draw_text(cr, width + 80, 8, m_label);
		else
			draw_text(cr, width, center_y - outer_radius - 8, m_label);
	}
	// draw value below		
	if (m_map) {
		if (m_view_type == SINGLE_DSP)
			draw_text(cr, width + 80, 24, (char*) m_map[m_value]);
		else
			draw_text(cr, width, center_y + outer_radius, (char*) m_map[m_value]);
	} else if (m_value_callback) {
		char vs[32];
		if (m_view_type == SINGLE_DSP)
			draw_text(cr, width + 80, 24, m_value_callback(m_value, vs, sizeof (vs)));
		else
			draw_text(cr, width, center_y + outer_radius, m_value_callback(m_value, vs, sizeof (vs)));
	}

	cr->set_line_width(.5);
	gdouble c;
	gdouble s;
	gint dx;
	gint dy;


	gint range = m_max - m_min;
	while (range < 100) range *= 10;
	while (range >= 1000) range /= 10;

	for (i = 0; i <= range; i += range / 20) {
		gdouble alpha = (gdouble) (i / (gdouble) range * 270.) - 45;
		c = cos(deg2rad(alpha));
		s = sin(deg2rad(alpha));

		dx = outer_radius * c;
		dy = outer_radius * s;

		gint dxi = inner_radius * c;
		gint dyi = inner_radius * s;

		cr->move_to(center_x - dxi, center_y - dyi);
		cr->line_to(center_x - dx, center_y - dy);
		cr->stroke();
	}
	// now draw a circle
	cr->set_line_width(.5);
	cr->save();
	cr->arc(center_x, center_y, inner_radius, 0.0, 2.0 * M_PI); // full circle
	if (get_sensitive())
		cr->set_source_rgba(m_b_red, m_b_green, m_b_blue, m_b_alpha);
	cr->fill_preserve();
	cr->restore(); // back to opaque black
	cr->stroke();

	if (m_value != m_default)
		cr->set_source_rgba(1.0, 0.3, 0.3, 1.0);
	else
		cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);

	gdouble alpha_value = (gdouble) (m_value / (gdouble) m_max * 270.) - 45;
	c = cos(deg2rad(alpha_value));
	s = sin(deg2rad(alpha_value));
	dx = inner_radius * c;
	dy = inner_radius * s;
	cr->set_line_width(2);
	cr->move_to(center_x, center_y);
	cr->line_to(center_x - dx, center_y - dy);
	cr->move_to(center_x, center_y);
	cr->arc(center_x, center_y, 2, 0.0, 2.0 * M_PI); // full circle
	cr->fill_preserve();
	cr->stroke();
	return true;
}

void ODial::draw_text(const Cairo::RefPtr<Cairo::Context>& cr,
		int rectangle_width, int rectangle_height, char* text) {
	Pango::FontDescription font;

	font.set_size(7 * Pango::SCALE);
	font.set_family("Sans");
	font.set_weight(Pango::WEIGHT_NORMAL);

	auto layout = create_pango_layout(text);

	layout->set_font_description(font);

	int text_width;
	int text_height;

	layout->get_pixel_size(text_width, text_height);

	cr->set_source_rgb(.8, .8, .8);
	cr->move_to((rectangle_width - text_width) / 2, rectangle_height);

	layout->show_in_cairo_context(cr);
}

void ODial::set_value(gint new_val) {
	m_value = new_val;
	if (m_value < 0)
		m_value = 0;
	if (m_value > m_max)
		m_value = m_max;
	queue_draw();
	signal_value_changed.emit();
}

void ODial::set_default(gint new_val) {
	m_default = new_val;
	if (m_default < m_min)
		m_default = m_min;
	if (m_default > m_max)
		m_default = m_max;

	m_value = m_default;
}

void ODial::set_scroll_step(gint new_val) {
	m_scroll_step = new_val;
}

void ODial::set_label(const char* label) {
	if (m_label)
		free(m_label);
	m_label = strdup(label);
}

void ODial::set_map(const char** map) {
	m_map = map;
}

void ODial::set_params(int mi, int ma, int def, int scroll_step) {
	set_min(mi);
	set_max(ma);
	set_default(def);
	set_scroll_step(scroll_step);
}

void ODial::set_knob_background_color(double red, double green, double blue, double alpha) {
	m_b_red = red;
	m_b_green = green;
	m_b_blue = blue;
	m_b_alpha = alpha;
}

void ODial::reset() {
	set_value(get_default());
}
