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

#include <gtkmm.h>

#include "ORoute.h"
#include "OMainWnd.h"

ORoute::ORoute() : Gtk::VBox() {
	
	m_grid.set_halign(Gtk::ALIGN_CENTER);
	m_grid.set_hexpand(true);
	m_label.set_label("Routing");
	
}

ORoute::~ORoute() {
}

void ORoute::init(OAlsa* alsa, Gtk::Window* wnd) {
	char l_title[64];
	int val;

	OMainWnd* wnd_ = (OMainWnd*) wnd;

	for (int i = 0; i < 8; i++) {
		m_route[i].set_name("route");
		m_route[i].set_margin_start(2);
		m_route[i].set_margin_bottom(2);
		
		m_route[i].append("Master Left");
		m_route[i].append("Master Right");
		for (int j = 0; j < 8; j++) {
			char entry[24];
			snprintf(entry, 24, "Output %d", j + 1);
			m_route[i].append(entry);
		}
	}
	for (int ri = 0; ri < 8; ri++) {
		val = alsa->getInteger(CTL_ROUTE, ri);
		m_route[ri].set_active(val);
		m_route[ri].signal_changed().connect(sigc::bind<>(sigc::mem_fun(alsa, &OAlsa::on_combo_control_changed), ri, CTL_ROUTE, &m_route[ri]));
	}
	add(m_label);
	add(m_grid);
}

void ORoute::set_view_type(VIEW_TYPE view_type) {

	if (view_type == HIDDEN) {
		std::vector<Gtk::Widget*> childList = m_grid.get_children();
		std::vector<Gtk::Widget*>::iterator it;

		for (it = childList.begin(); it < childList.end(); it++) {
			m_grid.remove(**it);
		}
	}
	if (view_type == NORMAL) {
		for (int i = 0; i < 8; i++) {
			m_grid.attach(m_route[i], 0, i, 3, 1);
		}
	}
	if (view_type == COMPACT) {
//		m_grid.attach(m_label, 0, 0, 2, 1);
		int k = 0;
		for (int i = 0; i < 4; i++) 
			for (int j = 1; j < 3; j++ ) 
				m_grid.attach(m_route[k++], j , i , 1, 1);
	}
}

void ORoute::reset(OAlsa* alsa) {

	for (int ri = 0; ri < 8; ri++) {
		m_route[ri].set_active(ri + (ri < 2 ? 0 : 2));
		usleep(RESET_VALUE_DELAY);
	}
}