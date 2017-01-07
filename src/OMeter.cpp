/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OMeter.cpp
 * Author: onkel
 * 
 * Created on January 3, 2017, 1:42 PM
 */

#include <gdkmm/drawable.h>
#include <gdkmm/general.h>  // for cairo helper functions

#include "OMeter.h"

//#define MIN(a,b) a < b ? a : b

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

void OMeter::on_size_request(Gtk::Requisition* requisition) {
	//Initialize the output parameter:
	*requisition = Gtk::Requisition();

	//Discover the total amount of minimum space needed by this widget.

	//Let's make this simple example widget always need 50 by 50:
	requisition->height = 50;
	requisition->width = 50;

}

void OMeter::on_size_allocate(Gtk::Allocation& allocation) {
	//Do something with the space that we have actually been given:
	//(We will not be given heights or widths less than we have requested, though
	//we might get more)

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
	//Do not call base class Gtk::Widget::on_realize().
	//It's intended only for widgets that set_has_window(false).

	set_realized();
	ensure_style();

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

		//Attach this widget's style to its Gdk::Window.
		style_attach();

		//set colors
		//		modify_bg(Gtk::STATE_NORMAL, Gdk::Color("black"));
		//		modify_fg(Gtk::STATE_NORMAL, Gdk::Color("green"));

		//make the widget receive expose events
		m_refGdkWindow->set_user_data(gobj());
	}
}

void OMeter::on_unrealize() {
	m_refGdkWindow.reset();

	//Call base class:
	Gtk::Widget::on_unrealize();
}

bool OMeter::on_expose_event(GdkEventExpose * event) {
	if (m_refGdkWindow) {
		gint i;
		gint width = get_allocation().get_width();
		int height = get_allocation().get_height();


		double scale = (double) level / 0x7fff;
		double pscale = (double) peak / 0x7fff;

		double hight = height * (direction == 1 ? 1 - scale : MIN(scale, .6));
		double hight1 = height * (direction == 1 ? 1 - scale : MIN(scale, .8));
		double hight2 = height * (direction == 1 ? 1 - scale : MIN(scale, .9));
		double hight3 = height * (direction == 1 ? 1 - scale : scale);
		gint center_x = width / 2;

		Cairo::RefPtr<Cairo::Context> cr = m_refGdkWindow->create_cairo_context();
		//		Gdk::Cairo::set_source_color(cr, Gdk::Color("green") );

		cr->set_source_rgba(m_b_red, m_b_green, m_b_blue, m_b_alpha);
		cr->set_line_width(width);

		//		painter.fillRect(left, myheight - 1, width, -myheight * min(scale, .6f), QColor(133, 177, 30, 255));
		//		if (scale > .6f)
		//			painter.fillRect(left, myheight * .4f, width, -myheight * min(scale - .6f, .2f), QColor(255, 255, 0, 255));
		//		if (scale > .8f)
		//			painter.fillRect(left, myheight * .2f, width, -myheight * min(scale - .8f, .2f), QColor(255, 128, 0, 255));
		//		if (scale > .9f)
		//			painter.fillRect(left, myheight * .1f, width, -myheight * min(scale - .9f, .1f), QColor(255, 0, 0, 255));
		//
		
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

		}

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
