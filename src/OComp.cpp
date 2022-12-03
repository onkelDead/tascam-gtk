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
#include <stdbool.h>

#include "config.h"

#ifdef HAVE_XML3
#include <libxml++-3.0/libxml++/libxml++.h>
#include <libxml++-3.0/libxml++/parsers/textreader.h>
#else
#include <libxml++-2.6/libxml++/libxml++.h>
#include <libxml++-2.6/libxml++/parsers/textreader.h>
#endif
#include <gtk-3.0/gtk/gtkenums.h>

#include "OComp.h"
#include "OMainWnd.h"


const char *cp_ration_map[] = {"1.0:1", "1.1:1", "1.3:1", "1.5:1", "1.7:1", "2.0:1", "2.5:1", "3.0:1", "3.5:1", "4.0:1", "5.0:1", "6.0:1", "8.0:1", "16.0:1", "inf:1"};

char* cp_threshold_text(int val, char* buf, size_t buf_size) {
	snprintf(buf, buf_size, "%ddb", val - 32);
	return buf;
}

char* cp_gain_text(int val, char* buf, size_t buf_size) {
	snprintf(buf, buf_size, val > 0 ? "+%ddb" : "%ddb", val);
	return buf;
}

char* cp_attack_text(int val, char* buf, size_t buf_size) {
	snprintf(buf, buf_size, "%dms", val + 2);
	return buf;
}

char* cp_release_text(int val, char* buf, size_t buf_size) {
	snprintf(buf, buf_size, "%dms", val * 10 + 10);
	return buf;
}

OComp::OComp() : Gtk::VBox() {
	m_view_type = HIDDEN;
	add(m_grid);
}

OComp::~OComp() {
}

void OComp::set_sensitive(bool val) {

	std::vector<Gtk::Widget*> childList = m_grid.get_children();
	std::vector<Gtk::Widget*>::iterator it;

	for (it = childList.begin(); it < childList.end(); it++) {
		Gtk::Widget* w = (*it);
		w->set_sensitive(val);
	}
	Gtk::VBox::set_sensitive(val);
}

void OComp::set_view_type(VIEW_TYPE view_type, CHANNEL_TYPE channel_type) {

	if (view_type == HIDDEN) {
		std::vector<Gtk::Widget*> childList = m_grid.get_children();
		std::vector<Gtk::Widget*>::iterator it;

		for (it = childList.begin(); it < childList.end(); it++) {
			m_grid.remove(**it);
		}
	}

	if (view_type == NORMAL) {
		m_enable->set_hexpand(false);
		m_enable->set_halign(Gtk::ALIGN_CENTER);

                m_grid.attach(*m_threshold, 0, 0, 1, 1);
                m_grid.attach(*m_gain, 1, 0, 1, 1);
                m_grid.attach(*m_attack, 0, 1, 1, 1);
                m_grid.attach(*m_release, 1, 1, 1, 1);
                m_grid.attach(*m_ratio, 0, 2, 1, 1);
                if (channel_type == MONO) {
			m_grid.attach(*m_enable, 0, 3, 1, 1);
			m_grid.attach(*m_reduction[0], 1, 2, 1, 2);
		}
		if (channel_type == STEREO) {
			m_grid.attach(*m_enable, 1, 2, 1, 1);
			m_grid.attach(*m_reduction[0], 2, 0, 1, 3);
			m_grid.attach(*m_reduction[1], 3, 0, 1, 3);
		}
	}

	if (view_type == COMPACT) {
		m_enable->set_hexpand(true);
		m_enable->set_halign(Gtk::ALIGN_FILL);
		m_grid.attach(*m_enable, 0, 0, 1, 1);
	}
	if (view_type == SINGLE_DSP) {
		m_grid.attach(*m_threshold, 0, 0, 1, 1);
		m_grid.attach(*m_ratio, 1, 0, 1, 1);
		m_grid.attach(*m_gain, 2, 0, 1, 1);
		m_grid.attach(*m_attack, 0, 1, 1, 1);
		m_grid.attach(*m_release, 1, 1, 1, 1);
		if (channel_type == MONO)
			m_grid.attach(*m_reduction[0], 3, 0, 1, 2);
		if (channel_type == STEREO) {
			m_grid.attach(*m_reduction[0], 3, 0, 1, 2);
			m_grid.attach(*m_reduction[1], 4, 0, 1, 2);
		}
	}

	m_view_type = view_type;
}

void OComp::set_ref_index(int index, Gtk::Window* wnd) {

	OMainWnd* wnd_ = (OMainWnd*) wnd;

	m_enable = &wnd_->m_comp_enable[index];
	m_enable->signal_toggled().connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_tb_changed), index, CTL_NAME_CP_ENABLE));

	m_threshold = &wnd_->m_threshold[index];
	m_threshold->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_CP_THRESHOLD));

	m_gain = &wnd_->m_gain[index];
	m_gain->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_CP_GAIN));

	m_attack = &wnd_->m_attack[index];
	m_attack->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_CP_ATTACK));

	m_release = &wnd_->m_release[index];
	m_release->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_CP_RELEASE));

	m_ratio = &wnd_->m_ratio[index];
	m_ratio->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_CP_RATIO));

	m_reduction[0] = &wnd_->m_reduction[index];
	if (!(index % 2) && index < 16)
		m_reduction[1] = &wnd_->m_reduction[index + 1];
}

void OComp::init(int index, OAlsa* alsa, Gtk::Window* wnd) {

	OMainWnd* wnd_ = (OMainWnd*) wnd;

	set_ref_index(index, wnd_);

	m_reduction[0]->set_hexpand(false);
	m_reduction[0]->set_margin_start(6);
	m_reduction[0]->set_halign(Gtk::ALIGN_CENTER);
	if (!(index % 2) && index < 16) {
		m_reduction[1]->set_hexpand(false);
		m_reduction[1]->set_margin_start(6);
		m_reduction[1]->set_halign(Gtk::ALIGN_CENTER);
	}

	m_enable->set_hexpand(false);
	m_enable->set_vexpand(false);
	m_enable->set_valign(Gtk::ALIGN_CENTER);
	if (index < NUM_CHANNELS)
		get_alsa_values(index, alsa);
}

void OComp::get_alsa_values(int index, OAlsa* alsa) {
	if (index < NUM_CHANNELS) {
		m_enable->set_active(alsa->getBoolean(CTL_NAME_CP_ENABLE, index));
		m_threshold->set_value(alsa->getInteger(CTL_NAME_CP_THRESHOLD, index));
		m_gain->set_value(alsa->getInteger(CTL_NAME_CP_GAIN, index));
		m_attack->set_value(alsa->getInteger(CTL_NAME_CP_ATTACK, index));
		m_release->set_value(alsa->getInteger(CTL_NAME_CP_RELEASE, index));
		m_ratio->set_value(alsa->getInteger(CTL_NAME_CP_RATIO, index));

	}
}

void OComp::reset(OAlsa* alsa, int index) {

	alsa->setBoolean(CTL_NAME_CP_ENABLE, 0, 0);
	m_enable->set_active(alsa->getBoolean(CTL_NAME_CP_ENABLE, 0));
	usleep(RESET_VALUE_DELAY);

	m_threshold->reset();
	usleep(RESET_VALUE_DELAY);

	m_gain->reset();
	usleep(RESET_VALUE_DELAY);

	m_attack->reset();
	usleep(RESET_VALUE_DELAY);

	m_release->reset();
	usleep(RESET_VALUE_DELAY);

	m_ratio->reset();
	usleep(RESET_VALUE_DELAY);

}

void OComp::save_values(FILE* file) {

	fprintf(file, "\t\t\t<enable>");
	fprintf(file, "%d", (int) m_enable->get_active());
	fprintf(file, "</enable>\n");

	fprintf(file, "\t\t\t<threshold>");
	fprintf(file, "%d", (int) m_threshold->get_value());
	fprintf(file, "</threshold>\n");

	fprintf(file, "\t\t\t<gain>");
	fprintf(file, "%d", (int) m_gain->get_value());
	fprintf(file, "</gain>\n");

	fprintf(file, "\t\t\t<attack>");
	fprintf(file, "%d", (int) m_attack->get_value());
	fprintf(file, "</attack>\n");

	fprintf(file, "\t\t\t<release>");
	fprintf(file, "%d", (int) m_release->get_value());
	fprintf(file, "</release>\n");

	fprintf(file, "\t\t\t<ratio>");
	fprintf(file, "%d", (int) m_ratio->get_value());
	fprintf(file, "</ratio>\n");

}

void OComp::load_values(Glib::ustring xml) {

	try {
		xmlpp::TextReader reader((const unsigned char*) xml.c_str(), xml.size());

		while (reader.read()) {
#ifdef HAVE_XML3
			if (!strcmp(reader.get_name().c_str(), "enable") && reader.get_node_type() != xmlpp::TextReader::NodeType::EndElement) {
#else
			if (!strcmp(reader.get_name().c_str(), "enable") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
#endif
				reader.read();
				m_enable->set_active(atoi(reader.get_value().c_str()) == 1);
				m_enable->toggled();
				usleep(RESET_VALUE_DELAY);
			}
#ifdef HAVE_XML3
			if (!strcmp(reader.get_name().c_str(), "threshold") && reader.get_node_type() != xmlpp::TextReader::NodeType::EndElement) {
#else
			if (!strcmp(reader.get_name().c_str(), "threshold") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
#endif
				reader.read();
				m_threshold->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
#ifdef HAVE_XML3
			if (!strcmp(reader.get_name().c_str(), "gain") && reader.get_node_type() != xmlpp::TextReader::NodeType::EndElement) {
#else
			if (!strcmp(reader.get_name().c_str(), "gain") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
#endif
				reader.read();
				m_gain->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
#ifdef HAVE_XML3
			if (!strcmp(reader.get_name().c_str(), "attack") && reader.get_node_type() != xmlpp::TextReader::NodeType::EndElement) {
#else
			if (!strcmp(reader.get_name().c_str(), "attack") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
#endif
				reader.read();
				m_attack->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
#ifdef HAVE_XML3
			if (!strcmp(reader.get_name().c_str(), "release") && reader.get_node_type() != xmlpp::TextReader::NodeType::EndElement) {
#else
			if (!strcmp(reader.get_name().c_str(), "release") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
#endif
				reader.read();
				m_release->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
#ifdef HAVE_XML3
			if (!strcmp(reader.get_name().c_str(), "ratio") && reader.get_node_type() != xmlpp::TextReader::NodeType::EndElement) {
#else
			if (!strcmp(reader.get_name().c_str(), "ratio") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
#endif
				reader.read();
				m_ratio->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return;
	}
}
#ifdef HAVE_OSC
void OComp::get_parameter_decriptor(int parameter_index, lo_message reply) {
	if (parameter_index == 0) {
		lo_message_add_int32(reply, 1); // plugin parameter index
		lo_message_add_string(reply, "Threshold"); // plugin parameter name
		int flags = 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, -32.); // lower
		lo_message_add_float(reply, 0.); // upper
		lo_message_add_string(reply, "%.0f dB"); // format string
		lo_message_add_int32(reply, 0); // scale points
		lo_message_add_double(reply, (double) m_threshold->get_value() - 32);
	}
	if (parameter_index == 1) {
		lo_message_add_int32(reply, 2); // plugin parameter index
		lo_message_add_string(reply, "Gain"); // plugin parameter name
		int flags = 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, 0.); // lower
		lo_message_add_float(reply, 20.); // upper
		lo_message_add_string(reply, "%.0f dB"); // format string
		lo_message_add_int32(reply, 0); // scale points
		lo_message_add_double(reply, (double) m_gain->get_value());
	}
	if (parameter_index == 2) {
		lo_message_add_int32(reply, 3); // plugin parameter index
		lo_message_add_string(reply, "Attack"); // plugin parameter name
		int flags = 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, 2.); // lower
		lo_message_add_float(reply, 200.); // upper
		lo_message_add_string(reply, "%.0f ms"); // format string
		lo_message_add_int32(reply, 0); // scale points
		lo_message_add_double(reply, (double) m_attack->get_value() + 2);
	}
	if (parameter_index == 3) {
		lo_message_add_int32(reply, 4); // plugin parameter index
		lo_message_add_string(reply, "Release"); // plugin parameter name
		int flags = 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, 1.); // lower
		lo_message_add_float(reply, 100.); // upper
		lo_message_add_string(reply, "%.0f0 ms"); // format string
		lo_message_add_int32(reply, 0); // scale points
		lo_message_add_double(reply, (double) (m_release->get_value() + 1));
	}
	if (parameter_index == 4) {
		lo_message_add_int32(reply, 5); // plugin parameter index
		lo_message_add_string(reply, "Ratio"); // plugin parameter name
		int flags = 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, 0); // lower
		lo_message_add_float(reply, 14); // upper
		lo_message_add_string(reply, ""); // format string
		lo_message_add_int32(reply, 15); // scale points

		for (int i = 0; i < 15; i++) {
			lo_message_add_float(reply, (float) i); // scale points
			lo_message_add_string(reply, cp_ration_map[i]); // format string
		}

		lo_message_add_double(reply, (double) m_ratio->get_value());
	}

}
#endif
