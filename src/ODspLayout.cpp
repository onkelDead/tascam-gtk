/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ODspLayout.cpp
 * Author: onkel
 * 
 * Created on January 16, 2017, 10:40 AM
 */

#include <iostream>

#include <gtkmm.h>
#include <libxml++-2.6/libxml++/libxml++.h>
#include <libxml++-2.6/libxml++/parsers/textreader.h>
#include "ODspLayout.h"
#include "OMainWnd.h"

ODspLayout::ODspLayout() : Gtk::VBox() {

	m_channel_type = MONO;
	set_halign(Gtk::ALIGN_FILL);
	add(m_grid);
	add(m_sep);
}

ODspLayout::ODspLayout(const ODspLayout& orig) {
}

ODspLayout::~ODspLayout() {
}

void ODspLayout::init(int index, OAlsa* alsa, Gtk::Window* wnd) {

	OMainWnd* wnd_ = (OMainWnd*) wnd;

	m_route = &wnd_->m_route;
	m_comp.init(index, alsa, wnd);
	m_eq.init(index, alsa, wnd);

}

void ODspLayout::set_channel_type(CHANNEL_TYPE num_channels) {
	m_channel_type = num_channels;
}

void ODspLayout::set_view_type(VIEW_TYPE view_type) {

	set_hexpand(true);
	set_halign(Gtk::ALIGN_FILL);
	m_grid.set_hexpand(true);
	m_grid.set_halign(Gtk::ALIGN_FILL);

	if (view_type == HIDDEN) {
		std::vector<Gtk::Widget*> childList = m_grid.get_children();
		std::vector<Gtk::Widget*>::iterator it;

		for (it = childList.begin(); it < childList.end(); it++) {
			m_grid.remove(**it);
		}
	}
	if (view_type == SINGLE_DSP) {
		m_grid.attach(m_eq, 0, 0, 1, 1);
		m_grid.attach(m_eq_sep, 1, 0, 1, 1);
		m_grid.attach(m_comp, 2, 0, 1, 1);
		m_grid.attach(*m_route, 3, 0, 1, 1);
	}
	if(view_type == PREPARE) {
		m_grid.attach(m_eq, 0, 0, 1, 1);
		m_grid.attach(m_eq_sep, 1, 0, 1, 1);
		m_grid.attach(m_comp, 2, 0, 1, 1);
		m_comp.set_view_type(SINGLE_DSP, m_channel_type);
		m_eq.set_view_type(SINGLE_DSP, m_channel_type);
	}
	else {
		m_comp.set_view_type(view_type, m_channel_type);
		m_eq.set_view_type(view_type, m_channel_type);
	}
}

void ODspLayout::set_ref_index(int index, Gtk::Window* wnd) {
	m_comp.set_ref_index(index, wnd);
	m_eq.set_ref_index(index, wnd);
}

void ODspLayout::set_sensitive(bool val) {
	m_eq.set_sensitive(val);
	m_comp.set_sensitive(val);
}