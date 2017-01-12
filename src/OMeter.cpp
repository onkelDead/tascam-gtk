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


//#include <gdkmm/drawable.h>
#include <gdkmm/general.h>  // for cairo helper functions

#include "OMeter.h"

#define WIDGET_WIDTH 10

OMeter::OMeter() :
Glib::ObjectBase("OMeter"),
Gtk::Widget() {
	set_has_window(true);
	level = 0;
	direction = 0;
	set_level_color(0, 1, 0, 1);
}

OMeter::~OMeter() {
}

Gtk::SizeRequestMode OMeter::get_request_mode_vfunc() const {
	//Accept the default value supplied by the base class.
	return Gtk::Widget::get_request_mode_vfunc();
}

void OMeter::get_preferred_width_vfunc(int& minimum_width, int& natural_width) const {
	minimum_width = WIDGET_WIDTH;
	natural_width = WIDGET_WIDTH;
}

void OMeter::get_preferred_height_for_width_vfunc(int /* width */,
		int& minimum_height, int& natural_height) const {
	minimum_height = 50;
	natural_height = 70;
}

void OMeter::get_preferred_height_vfunc(int& minimum_height, int& natural_height) const {
	minimum_height = 50;
	natural_height = 70;
}

void OMeter::get_preferred_width_for_height_vfunc(int /* height */,
		int& minimum_width, int& natural_width) const {
	minimum_width = WIDGET_WIDTH;
	natural_width = WIDGET_WIDTH;
}

void OMeter::on_size_allocate(Gtk::Allocation& allocation) {
	//Use the offered allocation for this container:
	set_allocation(allocation);

	if (m_refGdkWindow) {
		m_refGdkWindow->move_resize(allocation.get_x(), allocation.get_y(),
				allocation.get_width(), allocation.get_height());
	}
}

void OMeter::on_map() {
	//Call base class:
	Gtk::Widget::on_map();
}

void OMeter::on_unmap() {
	//Call base class:
	Gtk::Widget::on_unmap();
}

void OMeter::on_realize() {
	set_realized();
	//	ensure_style();

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

		attributes.event_mask = get_events() | GDK_EXPOSURE_MASK;
		attributes.window_type = GDK_WINDOW_CHILD;
		attributes.wclass = GDK_INPUT_OUTPUT;

		m_refGdkWindow = Gdk::Window::create(get_parent_window(), &attributes,
				GDK_WA_X | GDK_WA_Y);
		set_window(m_refGdkWindow);

		m_refGdkWindow->set_user_data(gobj());
	}
}

void OMeter::on_unrealize() {
	m_refGdkWindow.reset();

	//Call base class:
	Gtk::Widget::on_unrealize();
}

bool OMeter::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
	const Gtk::Allocation allocation = get_allocation();

	gint i;
	gint width = allocation.get_width();
	int height = allocation.get_height();
	double scale = (double) level / 0x7fff;
	double pscale = (double) peak / 0x7fff;

	double hight = height * (direction == 1 ? 1 - scale : MIN(scale, .6));
	double hight1 = height * (direction == 1 ? 1 - scale : MIN(scale, .8));
	double hight2 = height * (direction == 1 ? 1 - scale : MIN(scale, .9));
	double hight3 = height * (direction == 1 ? 1 - scale : scale);
	gint center_x = width / 2;

	cr->set_source_rgba(m_b_red, m_b_green, m_b_blue, m_b_alpha);
	cr->set_line_width(width);

	double l1 = .55;
	double l2 = .75;
	double l3 = .83;

	if (direction == 1) {
		cr->move_to(center_x, hight);
		cr->line_to(center_x, 0);
		cr->stroke();
	} else {
		cr->move_to(center_x, height);
		cr->line_to(center_x, height - height * MIN(scale, l1));
		cr->stroke();

		if (scale > l1) {
			cr->set_source_rgba(0, 1, 0, m_b_alpha);
			cr->move_to(center_x, height - height * l1);
			cr->line_to(center_x, height - height * MIN(scale, l2));
			cr->stroke();
		}
		if (scale > l2) {
			cr->set_source_rgba(1, 1, 0, m_b_alpha);
			cr->move_to(center_x, height - height * l2);
			cr->line_to(center_x, height - height * MIN(scale, l3));
			cr->stroke();
		}
		if (scale > l3) {
			cr->set_source_rgba(1, 0, 0, m_b_alpha);
			cr->move_to(center_x, height - height * l3);
			cr->line_to(center_x, height - height * scale);
			cr->stroke();
		}
		cr->set_source_rgba(0, 0, 0, 1);
		cr->move_to(center_x, height - height * scale);
		cr->line_to(center_x, 0);
		cr->stroke();
		
		
	}

	return true;

}

void OMeter::setLevel(int val) {
	if (direction == 0) {
		if (val >= level) {
			level = val;
			peak_count = 50;
			peak = val;
		} else {
			level = level - (level - val) / 20;
			peak_count = peak_count ? peak_count - 1 : 0;
		}
	} else {
		if (val <= level) {
			level = val;
			peak_count = 50;
			peak = val;
		} else {
			level = level + (val - level) / 20;
			peak_count = peak_count ? peak_count - 1 : 0;
		}
	}
	queue_draw();
}

void OMeter::set_level_direction(int direct) {
	direction = direct;
}

void OMeter::set_level_color(double red, double green, double blue, double alpha) {
	m_b_red = red;
	m_b_green = green;
	m_b_blue = blue;
	m_b_alpha = alpha;
}
