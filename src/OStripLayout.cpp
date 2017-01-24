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


#include <iostream>

#include <gtkmm.h>
#include <libxml++-2.6/libxml++/libxml++.h>
#include <libxml++-2.6/libxml++/parsers/textreader.h>
#include "OStripLayout.h"
#include "OMainWnd.h"

OStripLayout::OStripLayout() : Gtk::VBox() {

	m_event_box.add(m_title);
	m_event_box.add_events(Gdk::EventMask::BUTTON_PRESS_MASK);

	m_DspEnable.set_label("Active");
	m_DspEnable.set_name("dsp-active");

	m_channel_type = MONO;


	//	m_grid.attach(m_sep, 1, 0, 1, 8);

	add(m_grid);
}

OStripLayout::~OStripLayout() {
}

void OStripLayout::init(int index, OAlsa* alsa, Gtk::Window* wnd) {
	OMainWnd* wnd_ = (OMainWnd*) wnd;
	char l_title[64];
	int val;

	snprintf(l_title, sizeof (l_title), "Ch %d", index + 1);
	m_title.set_text(l_title);

	m_comp.init(index, alsa, wnd);
	m_eq.init(index, alsa, wnd);
	m_fader.init(index, alsa, wnd);

	m_DspEnable.signal_toggled().connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_tb_changed), index, CTL_NAME_CHANNEL_ACTIVE));

}

void OStripLayout::set_view_type(VIEW_TYPE view_type) {

	m_comp.set_view_type(view_type, m_channel_type);
	m_eq.set_view_type(view_type, m_channel_type);
	m_fader.set_view_type(view_type, m_channel_type);

	if (view_type == HIDDEN) {
		std::vector<Gtk::Widget*> childList = m_grid.get_children();
		std::vector<Gtk::Widget*>::iterator it;

		for (it = childList.begin(); it < childList.end(); it++) {
			m_grid.remove(**it);
		}
	}

	if (view_type == NORMAL) {
		m_grid.attach(m_event_box, 0, 0, 1, 1);
		m_grid.attach(m_comp, 0, 3, 1, 1);
		m_grid.attach(m_comp_sep, 0, 4, 1, 1);
		m_grid.attach(m_eq, 0, 5, 1, 1);
		m_grid.attach(m_eq_sep, 0, 6, 1, 1);

		m_grid.attach(m_fader, 0, 7, 1, 1);

		if (m_DspEnable.get_parent())
			m_grid.remove(m_DspEnable);
		m_grid.attach(m_sep, 1, 0, 1, 8);

	}

	if (view_type == COMPACT) {
		m_grid.attach(m_event_box, 0, 0, 1, 1);
		m_grid.attach(m_comp, 0, 3, 1, 1);
		m_grid.attach(m_eq, 0, 5, 1, 1);

		m_grid.attach(m_fader, 0, 7, 1, 1);
		if (!m_DspEnable.get_parent())
			m_grid.attach(m_DspEnable, 0, 1, 1, 1);
		m_grid.attach(m_sep, 1, 0, 1, 8);
	}
	show_all_children(true);
}

void OStripLayout::set_channel_type(CHANNEL_TYPE num_channels) {
	m_channel_type = num_channels;
}

void OStripLayout::reset(OAlsa* alsa, int index) {

	m_comp.reset(alsa, index);
	m_eq.reset(alsa, index);
	m_fader.reset(alsa, index);
}

void OStripLayout::save_values(FILE* file, int indent) {

	//	fprintf(file, "\t\t<type>%d</type>\n", (int)m_channel_type);


	fprintf(file, "\t\t<compressor>\n");
	m_comp.save_values(file);
	fprintf(file, "\t\t</compressor>\n");

	fprintf(file, "\t\t<equalizer>\n");
	m_eq.save_values(file);
	fprintf(file, "\t\t</equalizer>\n");

	fprintf(file, "\t\t<fader>\n");
	m_fader.save_values(file);
	fprintf(file, "\t\t</fader>\n");

}

void OStripLayout::load_values(Glib::ustring xml) {

	try {
		xmlpp::TextReader reader((const unsigned char*) xml.c_str(), xml.size());

		while (reader.read()) {

			if (!strcmp(reader.get_name().c_str(), "compressor") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				m_comp.load_values(reader.read_outer_xml());
			}
			if (!strcmp(reader.get_name().c_str(), "equalizer") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				m_eq.load_values(reader.read_outer_xml());
			}
			if (!strcmp(reader.get_name().c_str(), "fader") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				m_fader.load_values(reader.read_outer_xml());
			}
		}

	} catch (const std::exception& e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return;
	}
}

