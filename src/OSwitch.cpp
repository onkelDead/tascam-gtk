/*
  Copyright 2024 Detlef Urban <onkel@paraair.de>

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

#include <gdkmm/general.h>  // for cairo helper functions
#include <iostream>
#include <cstring>

#include "OSwitch.h"

OSwitch::OSwitch() : 
Glib::ObjectBase("ODial"),
m_label(0),
m_value_callback(0),
Gtk::Widget() {
    set_has_window(true);
    m_fontsize = 10;
    m_ledsize = 8;
    m_active = 0;
    set_size_request(80, m_ledsize * 4);
    set_ledcolor(0.6, 0.6, 0.6, 1);
    set_halign(Gtk::ALIGN_FILL);
    set_hexpand(true);
    set_name("o-switch");    
    set_sensitive(true);
}

OSwitch::~OSwitch() {
}

Gtk::SizeRequestMode OSwitch::get_request_mode_vfunc() const {
	//Accept the default value supplied by the base class.
	return Gtk::Widget::get_request_mode_vfunc();
}

void OSwitch::get_preferred_width_vfunc(int& minimum_width, int& natural_width) const {
	minimum_width = m_ledsize * 2 + m_textwidth + 8;
	natural_width = minimum_width;
}

void OSwitch::get_preferred_height_for_width_vfunc(int /* width */,
		int& minimum_height, int& natural_height) const {
	minimum_height = m_ledsize * 2 + 2;
	natural_height = minimum_height;
}

void OSwitch::get_preferred_height_vfunc(int& minimum_height, int& natural_height) const {
	minimum_height = m_ledsize * 2 + 2;
	natural_height = minimum_height;
}

void OSwitch::get_preferred_width_for_height_vfunc(int /* height */,
		int& minimum_width, int& natural_width) const {
	minimum_width = m_ledsize * 2 + m_textwidth + 8;
	natural_width = minimum_width;
}

void OSwitch::on_size_allocate(Gtk::Allocation& allocation) {
	set_allocation(allocation);

	if (m_refGdkWindow) {
		m_refGdkWindow->move_resize(allocation.get_x(), allocation.get_y(),
				allocation.get_width(), allocation.get_height());
	}
}

void OSwitch::on_map() {
	Gtk::Widget::on_map();
}

void OSwitch::on_unmap() {
	Gtk::Widget::on_unmap();
}

void OSwitch::on_realize() {
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

void OSwitch::on_unrealize() {
	m_refGdkWindow.reset();

	Gtk::Widget::on_unrealize();
}


bool OSwitch::on_button_press_event(GdkEventButton* event) {
	if (event->button == 1) {
                m_active = !m_active;
                signal_switched.emit();
                queue_draw();
		return true;
	}
	if (event->button == 3) {
		return true;
	}

	return false;
}

bool OSwitch::on_button_release_event(GdkEventButton* event) {
	if (event->button == 1) {
		return true;
	}

	return false;
}

bool OSwitch::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
	const Gtk::Allocation allocation = get_allocation();

	gint i;
	gint width = m_view_type == SINGLE_DSP ? 40 : allocation.get_width();
	int height = allocation.get_height();

        int center_x = width / 2;
	gint center_y = /* m_view_type == SINGLE_DSP ? 20 :*/ (height) / 2;

	gint led_radius = m_ledsize;

        int content_width = m_ledsize * 2 + 8 + m_textwidth;
        int content_heigth = std::max(8 + m_ledsize * 2, m_textheight);
        
	// draw switch title		
	if (m_label) {
            Pango::FontDescription font;
            auto layout = create_pango_layout(m_label);
            
            font.set_size(m_fontsize * Pango::SCALE);
            font.set_family("Sans");
            font.set_weight(Pango::WEIGHT_NORMAL);
            layout->set_font_description(font);
            
            int text_center_x = m_align ==  Gtk::Align::ALIGN_START ?
                m_ledsize * 2 + 8 + m_textwidth / 2 :
                center_x - (content_width / 2) + m_ledsize * 2 + 8 + m_textwidth / 2;
            cr->set_source_rgba(1., 1., 1., m_active ? m_b_alpha : 1.);
            cr->move_to(text_center_x - m_textwidth / 2, center_y - m_textheight / 2 + 2);
            layout->show_in_cairo_context(cr);
	}

        // now draw a led
        int led_center_x = m_align ==  Gtk::Align::ALIGN_START ?
            4 + m_ledsize :
            center_x - (content_width / 2) + 4 + m_ledsize;
	cr->set_line_width(.5);
        cr->move_to(led_center_x + m_ledsize, center_y);
	cr->arc(led_center_x, center_y, led_radius, 0.0, 2.0 * M_PI); // full circle
	if (get_sensitive())
		cr->set_source_rgba(m_b_red, m_b_green, m_b_blue, m_active ? m_b_alpha : 0.1);
	if (is_sensitive())
            cr->fill_preserve();
	cr->stroke();
        
        cr->set_line_width(1.);
        cr->set_source_rgba(.5, .5, .5, m_active ? .4 : .2);
        cr->move_to(0,0);
        cr->line_to(0, height);
        cr->line_to(width, height);
        cr->set_source_rgba(.5, .5, .5, m_active ? .2 : .4);
        cr->line_to(width, 0);
        cr->line_to(0, 0);
        cr->stroke();
	return true;
}

void OSwitch::set_label(const char* label) {
	if (m_label) {
		free(m_label);
                m_label = NULL;
                
        }
        if (label) {
            int text_width;
            int text_height;
            Pango::FontDescription font;
            m_label = strdup(label);

            Glib::RefPtr<Pango::Layout> layout = create_pango_layout(m_label);

            font.set_size(m_fontsize * Pango::SCALE);
            font.set_family("Sans");
            font.set_weight(Pango::WEIGHT_NORMAL);
            layout->set_font_description(font);

            layout->get_pixel_size(text_width, text_height);

            m_textheight = text_height;
            m_textwidth = text_width;
        }
}

void OSwitch::set_ledcolor(double red, double green, double blue, double alpha) {
	m_b_red = red;
	m_b_green = green;
	m_b_blue = blue;
	m_b_alpha = alpha;
}

void OSwitch::set_ledsize(int ls) {
    m_ledsize = ls;
    set_size_request(40, m_ledsize * 2 + 8);
}

void OSwitch::set_fontsize(int fs) {
    m_fontsize = fs;
    queue_draw();
}
    
void OSwitch::set_align(Gtk::Align align) {
    m_align = align;
}