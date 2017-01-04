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

OMeter::OMeter() :
Glib::ObjectBase("OMeter"),
Gtk::Widget() {
	set_has_window(true);
	level = 0;
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

		double hight = height * scale;
		gint center_x = width / 2;
		Cairo::RefPtr<Cairo::Context> cr = m_refGdkWindow->create_cairo_context();
		Gdk::Cairo::set_source_color(cr, Gdk::Color("green") );
		cr->set_line_width(width);

		cr->move_to(center_x , height - hight);
		cr->line_to(center_x , height);
		cr->stroke();		
		
	}
	return true;
}

void OMeter::setLevel(int val) {
	if (val >= level) {
		level = val;
		peak_count = 50;
		peak = val;
	} else {
		level = level - (level - val) / 20;
		peak_count = peak_count ? peak_count - 1 : 0;
	}
	queue_draw();
}
