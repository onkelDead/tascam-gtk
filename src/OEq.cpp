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
#include <vector>

#include <gtkmm.h>
#include <stdbool.h>
#include <libxml++-2.6/libxml++/libxml++.h>
#include <libxml++-2.6/libxml++/parsers/textreader.h>

#include "OEq.h"
#include "ODial.h"
#include "OMainWnd.h"

const char *eq_low_freq_map[] = {"32", "40", "50", "60", "70", "80", "90", "100", "125", "150", "175", "200", "225", "250",
	"300", "350", "400", "450", "500", "600", "700", "800", "850", "900", "950", "1.0k",
	"1.1k", "1.2k", "1.3k", "1.4k", "1.5k", "1.6k"};
const int eq_low_freq_map_size = 32;


const char *eq_high_freq_map[] = {"1.7k", "1.8k", "1.9k", "2.0k", "2.2k", "2.4k", "2.6k", "2.8k", "3.0k", "3.2k", "3.4k", "3.6k", "3.8k", "4.0k",
	"4.5k", "5.0k", "5.5k", "6.0k", "6.5k", "7.0k", "7.5k", "8.0k", "8.5k",
	"9.0k", "10k", "11k", "12k", "13k", "14k", "15k", "16k", "17k", "18k"};
const int eq_high_freq_map_size = 33;

char* eq_width_text(int val, char* buf, size_t buf_size) {
	int a = (1 << val);
	float b = a;
	float c = b / 4;
	snprintf(buf, buf_size, "Q%2.2f", c);
	return buf;
}

char* eq_level_text(int val, char* buf, size_t buf_size) {
	int v = val - 12;
	snprintf(buf, buf_size, v > 0 ? "+%ddB" : "%ddB", v);
	return buf;
}

char* eq_high_freq_text(int val, char* buf, size_t buf_size) {
	snprintf(buf, buf_size, "%sHz", eq_high_freq_map[val]);
	return buf;
}

char* eq_lowhigh_freq_text(int val, char* buf, size_t buf_size) {

	if (val < eq_low_freq_map_size)
		snprintf(buf, buf_size, "%sHz", eq_low_freq_map[val]);
	else
		snprintf(buf, buf_size, "%sHz", eq_high_freq_map[val - eq_low_freq_map_size]);
	return buf;
}

char* eq_low_freq_text(int val, char* buf, size_t buf_size) {
	snprintf(buf, buf_size, "%sHz", eq_low_freq_map[val]);
	return buf;
}

#define EBLUE_NORMAL .5, .55, 1., 1.
#define EBLUE_LIGHT .78, .8, 1., 1.

void OEq::set_sensitive(bool val){
	m_is_active = val;

	std::vector<Gtk::Widget*> childList = m_grid.get_children();
	std::vector<Gtk::Widget*>::iterator it;

	for (it = childList.begin(); it < childList.end(); it++) {
		Gtk::Widget* w = (*it);
		w->set_sensitive(val);
	}
}

OEq::OEq() : Gtk::VBox() {
	m_view_type = HIDDEN;
	add(m_grid);
}

OEq::~OEq() {
}

void OEq::set_view_type(VIEW_TYPE view_type, CHANNEL_TYPE channel_type) {

	if (view_type == HIDDEN) {
		std::vector<Gtk::Widget*> childList = m_grid.get_children();
		std::vector<Gtk::Widget*>::iterator it;

		for (it = childList.begin(); it < childList.end(); it++) {
			m_grid.remove(**it);
		}
	}

	if (view_type == NORMAL) {
		m_eq_enable->set_hexpand(false);
		m_eq_enable->set_halign(Gtk::ALIGN_CENTER);

		if (channel_type == MONO) {
			m_grid.attach(*m_high_freq_gain, 0, 0, 1, 1);
			m_grid.attach(*m_high_freq_band, 1, 0, 1, 1);
			m_grid.attach(*m_mid_high_freq_gain, 0, 1, 1, 1);
			m_grid.attach(*m_mid_high_freq_band, 1, 1, 1, 1);
			m_grid.attach(*m_mid_high_freq_width, 0, 2, 2, 1);
			m_grid.attach(*m_mid_low_freq_gain, 0, 3, 1, 1);
			m_grid.attach(*m_mid_low_freq_band, 1, 3, 1, 1);
			m_grid.attach(*m_mid_low_freq_width, 0, 4, 1, 1);
			m_grid.attach(*m_eq_enable, 1, 4, 1, 1);
			m_grid.attach(*m_low_freq_gain, 0, 5, 1, 1);
			m_grid.attach(*m_low_freq_band, 1, 5, 1, 1);
		}
		if (channel_type == STEREO) {
			m_grid.attach(*m_high_freq_gain, 0, 0, 1, 1);
			m_grid.attach(*m_mid_high_freq_gain, 0, 1, 1, 1);
			m_grid.attach(*m_mid_low_freq_gain, 0, 2, 1, 1);
			m_grid.attach(*m_low_freq_gain, 0, 3, 1, 1);

			m_grid.attach(*m_high_freq_band, 1, 0, 1, 1);
			m_grid.attach(*m_mid_high_freq_band, 1, 1, 1, 1);
			m_grid.attach(*m_mid_low_freq_band, 1, 2, 1, 1);
			m_grid.attach(*m_low_freq_band, 1, 3, 1, 1);

			m_grid.attach(*m_mid_high_freq_width, 2, 1, 1, 1);
			m_grid.attach(*m_mid_low_freq_width, 2, 2, 1, 1);
			m_grid.attach(*m_eq_enable, 2, 3, 1, 1);
		}

	}
	if (view_type == COMPACT) {
		m_eq_enable->set_hexpand(true);
		m_eq_enable->set_halign(Gtk::ALIGN_FILL);
		m_grid.attach(*m_eq_enable, 0, 0, 1, 1);

	}
	if (view_type == SINGLE_DSP) {
		m_grid.attach(*m_high_freq_gain, 0, 0, 1, 1);
		m_grid.attach(*m_high_freq_band, 1, 0, 1, 1);
		m_grid.attach(*m_mid_high_freq_gain, 3, 0, 1, 1);
		m_grid.attach(*m_mid_high_freq_band, 4, 0, 1, 1);
		m_grid.attach(*m_mid_high_freq_width, 5, 0, 1, 1);
		m_grid.attach(*m_mid_low_freq_gain, 0, 1, 1, 1);
		m_grid.attach(*m_mid_low_freq_band, 1, 1, 1, 1);
		m_grid.attach(*m_mid_low_freq_width, 2, 1, 1, 1);
		//		m_grid.attach(*m_eq_enable, 1, 4, 1, 1);
		m_grid.attach(*m_low_freq_gain, 3, 1, 1, 1);
		m_grid.attach(*m_low_freq_band, 4, 1, 1, 1);
	}

	m_eq_enable->set_vexpand(false);
	m_eq_enable->set_valign(Gtk::ALIGN_CENTER);
	
	m_view_type = view_type;
}



void OEq::init(int index, OAlsa* alsa, Gtk::Window* wnd) {

	set_ref_index(index, wnd);
	if (index < NUM_CHANNELS) {
		get_alsa_values(index, alsa);
	}
	
}

void OEq::set_ref_index(int index, Gtk::Window* wnd){
	
	OMainWnd* wnd_ = (OMainWnd*) wnd;

	m_eq_enable = &wnd_->m_eq_enable[index];
	m_eq_enable->signal_toggled().connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_tb_changed), index, CTL_NAME_EQ_ENABLE));
	
	m_high_freq_gain = &wnd_->m_high_freq_gain[index];
	m_high_freq_gain->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_EQ_HIGH_LEVEL));
	
	m_high_freq_band = &wnd_->m_high_freq_band[index];
	m_high_freq_band->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_EQ_HIGH_FREQ));
	
	m_mid_high_freq_gain = &wnd_->m_mid_high_freq_gain[index];
	m_mid_high_freq_gain->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_EQ_MIDHIGH_LEVEL));
	
	m_mid_high_freq_band = &wnd_->m_mid_high_freq_band[index];
	m_mid_high_freq_band->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_EQ_MIDHIGH_FREQ));
	
	m_mid_high_freq_width = &wnd_->m_mid_high_freq_width[index];
	m_mid_high_freq_width->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_EQ_MIDHIGHWIDTH_FREQ));
	
	m_mid_low_freq_gain = &wnd_->m_mid_low_freq_gain[index];
	m_mid_low_freq_gain->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_EQ_MIDLOW_LEVEL));
	
	m_mid_low_freq_band = &wnd_->m_mid_low_freq_band[index];
	m_mid_low_freq_band->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_EQ_MIDLOW_FREQ));
	
	m_mid_low_freq_width = &wnd_->m_mid_low_freq_width[index];
	m_mid_low_freq_width->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_EQ_MIDLOWWIDTH_FREQ));
	
	m_low_freq_gain = &wnd_->m_low_freq_gain[index];
	m_low_freq_gain->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_EQ_LOW_LEVEL));
	
	m_low_freq_band = &wnd_->m_low_freq_band[index];
	m_low_freq_band->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_EQ_LOW_FREQ));
}

void OEq::get_alsa_values(int channel_index, OAlsa* alsa) {
	m_eq_enable->set_active(alsa->getBoolean(CTL_NAME_EQ_ENABLE, channel_index));
	m_high_freq_gain->set_value(alsa->getInteger(CTL_NAME_EQ_HIGH_LEVEL, channel_index));
	m_high_freq_band->set_value(alsa->getInteger(CTL_NAME_EQ_HIGH_FREQ, channel_index));
	m_mid_high_freq_gain->set_value(alsa->getInteger(CTL_NAME_EQ_MIDHIGH_LEVEL, channel_index));
	m_mid_high_freq_band->set_value(alsa->getInteger(CTL_NAME_EQ_MIDHIGH_FREQ, channel_index));
	m_mid_high_freq_width->set_value(alsa->getInteger(CTL_NAME_EQ_MIDHIGHWIDTH_FREQ, channel_index));
	m_mid_low_freq_gain->set_value(alsa->getInteger(CTL_NAME_EQ_MIDLOW_LEVEL, channel_index));
	m_mid_low_freq_band->set_value(alsa->getInteger(CTL_NAME_EQ_MIDLOW_FREQ, channel_index));
	m_mid_low_freq_width->set_value(alsa->getInteger(CTL_NAME_EQ_MIDLOWWIDTH_FREQ, channel_index));
	m_low_freq_gain->set_value(alsa->getInteger(CTL_NAME_EQ_LOW_LEVEL, channel_index));
	m_low_freq_band->set_value(alsa->getInteger(CTL_NAME_EQ_LOW_FREQ, channel_index));
}

void OEq::reset(OAlsa* alsa, int index) {

	alsa->setBoolean(CTL_NAME_EQ_ENABLE, 0, 0);
	m_eq_enable->set_active(alsa->getBoolean(CTL_NAME_MUTE, 0));
	usleep(RESET_VALUE_DELAY);

	m_high_freq_gain->reset();
	usleep(RESET_VALUE_DELAY);

	m_high_freq_band->reset();
	usleep(RESET_VALUE_DELAY);

	m_mid_high_freq_gain->reset();
	usleep(RESET_VALUE_DELAY);

	m_mid_high_freq_band->reset();
	usleep(RESET_VALUE_DELAY);

	m_mid_high_freq_width->reset();
	usleep(RESET_VALUE_DELAY);

	m_mid_low_freq_gain->reset();
	usleep(RESET_VALUE_DELAY);

	m_mid_low_freq_band->reset();
	usleep(RESET_VALUE_DELAY);

	m_mid_low_freq_width->reset();
	usleep(RESET_VALUE_DELAY);

	m_low_freq_gain->reset();
	usleep(RESET_VALUE_DELAY);

	m_low_freq_band->reset();
	usleep(RESET_VALUE_DELAY);
}

void OEq::save_values(FILE * file) {

	fprintf(file, "\t\t\t<enable>");
	fprintf(file, "%d", (int) m_eq_enable->get_active());
	fprintf(file, "</enable>\n");

	fprintf(file, "\t\t\t<high_freq_gain>");
	fprintf(file, "%d", (int) m_high_freq_gain->get_value());
	fprintf(file, "</high_freq_gain>\n");

	fprintf(file, "\t\t\t<high_freq_band>");
	fprintf(file, "%d", (int) m_high_freq_band->get_value());
	fprintf(file, "</high_freq_band>\n");

	fprintf(file, "\t\t\t<mid_high_freq_gain>");
	fprintf(file, "%d", (int) m_mid_high_freq_gain->get_value());
	fprintf(file, "</mid_high_freq_gain>\n");

	fprintf(file, "\t\t\t<mid_high_freq_band>");
	fprintf(file, "%d", (int) m_mid_high_freq_band->get_value());
	fprintf(file, "</mid_high_freq_band>\n");

	fprintf(file, "\t\t\t<mid_high_freq_width>");
	fprintf(file, "%d", (int) m_mid_high_freq_width->get_value());
	fprintf(file, "</mid_high_freq_width>\n");

	fprintf(file, "\t\t\t<mid_low_freq_gain>");
	fprintf(file, "%d", (int) m_mid_low_freq_gain->get_value());
	fprintf(file, "</mid_low_freq_gain>\n");

	fprintf(file, "\t\t\t<mid_low_freq_band>");
	fprintf(file, "%d", (int) m_mid_low_freq_band->get_value());
	fprintf(file, "</mid_low_freq_band>\n");

	fprintf(file, "\t\t\t<mid_low_freq_width>");
	fprintf(file, "%d", (int) m_mid_low_freq_width->get_value());
	fprintf(file, "</mid_low_freq_width>\n");

	fprintf(file, "\t\t\t<low_freq_gain>");
	fprintf(file, "%d", (int) m_low_freq_gain->get_value());
	fprintf(file, "</low_freq_gain>\n");

	fprintf(file, "\t\t\t<low_freq_band>");
	fprintf(file, "%d", (int) m_low_freq_band->get_value());
	fprintf(file, "</low_freq_band>\n");

}

void OEq::load_values(Glib::ustring xml) {

	try {
		xmlpp::TextReader reader((const unsigned char*) xml.c_str(), xml.size());

		while (reader.read()) {
			if (!strcmp(reader.get_name().c_str(), "enable") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_eq_enable->set_active(atoi(reader.get_value().c_str()) == 1);
				m_eq_enable->toggled();
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "high_freq_gain") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_high_freq_gain->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "high_freq_band") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_high_freq_band->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "mid_high_freq_gain") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_mid_high_freq_gain->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "mid_high_freq_band") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_mid_high_freq_band->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "mid_high_freq_width") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_mid_high_freq_width->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "mid_low_freq_gain") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_mid_low_freq_gain->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "mid_low_freq_band") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_mid_low_freq_band->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "mid_low_freq_width") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_mid_low_freq_width->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "low_freq_gain") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_low_freq_gain->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "low_freq_band") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_low_freq_band->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}

		}
	} catch (const std::exception& e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return;
	}
}

void OEq::get_parameter_decriptor(int parameter_index, lo_message reply) {
	if (parameter_index == 0) {
		lo_message_add_int32(reply, parameter_index + 1); // plugin parameter index
		lo_message_add_string(reply, "High gain"); // plugin parameter name
		int flags = 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, -12.); // lower
		lo_message_add_float(reply, 12.); // upper
		lo_message_add_string(reply, "%.0f dB"); // format string
		lo_message_add_int32(reply, 0); // scale points
		lo_message_add_double(reply, (double) m_high_freq_gain->get_value() - 12);
	}
	if (parameter_index == 1) {
		lo_message_add_int32(reply, parameter_index + 1); // plugin parameter index
		lo_message_add_string(reply, "High Freq"); // plugin parameter name
		int flags = 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, 0.); // lower
		lo_message_add_float(reply, 31.); // upper
		lo_message_add_string(reply, ""); // format string
		lo_message_add_int32(reply, eq_high_freq_map_size); // scale points
		for (int i = 0; i < eq_high_freq_map_size; i++) {
			lo_message_add_float(reply, (float) i); // scale points
			lo_message_add_string(reply, eq_high_freq_map[i]); // format string
		}
		lo_message_add_double(reply, (double) m_high_freq_band->get_value());
	}
	if (parameter_index == 2) {
		lo_message_add_int32(reply, parameter_index + 1); // plugin parameter index
		lo_message_add_string(reply, "Mid High gain"); // plugin parameter name
		int flags = 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, -12.); // lower
		lo_message_add_float(reply, 12.); // upper
		lo_message_add_string(reply, "%.0f dB"); // format string
		lo_message_add_int32(reply, 0); // scale points
		lo_message_add_double(reply, (double) m_mid_high_freq_gain->get_value() - 12);
	}
	if (parameter_index == 3) {
		lo_message_add_int32(reply, parameter_index + 1); // plugin parameter index
		lo_message_add_string(reply, "Mid High Freq"); // plugin parameter name
		int flags = 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, 0.); // lower
		lo_message_add_float(reply, 64.); // upper
		lo_message_add_string(reply, ""); // format string
		lo_message_add_int32(reply, eq_high_freq_map_size + eq_low_freq_map_size); // scale points
		for (int i = 0; i < eq_high_freq_map_size + eq_low_freq_map_size; i++) {
			lo_message_add_float(reply, (float) i); // scale points
			lo_message_add_string(reply, i < eq_low_freq_map_size ? eq_low_freq_map[i] : eq_high_freq_map[i - eq_low_freq_map_size]); // format string
		}
		lo_message_add_double(reply, (double) m_mid_high_freq_band->get_value());
	}
	if (parameter_index == 4) {
		lo_message_add_int32(reply, parameter_index + 1); // plugin parameter index
		lo_message_add_string(reply, "Mid High Q"); // plugin parameter name
		int flags = 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, 0.); // lower
		lo_message_add_float(reply, 6.); // upper
		lo_message_add_string(reply, ""); // format string
		lo_message_add_int32(reply, 7); // scale points
		for (int i = 0; i < 7; i++) {
			char sp[16];
			lo_message_add_float(reply, (float) i); // scale points
			lo_message_add_string(reply, eq_width_text(i, sp, 16)); // format string
		}
		lo_message_add_double(reply, (double) m_mid_high_freq_width->get_value());
	}
	if (parameter_index == 5) {
		lo_message_add_int32(reply, parameter_index + 1); // plugin parameter index
		lo_message_add_string(reply, "Mid Low gain"); // plugin parameter name
		int flags = 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, -12.); // lower
		lo_message_add_float(reply, 12.); // upper
		lo_message_add_string(reply, "%.0f dB"); // format string
		lo_message_add_int32(reply, 0); // scale points
		lo_message_add_double(reply, (double) m_mid_low_freq_gain->get_value() - 12);
	}
	if (parameter_index == 6) {
		lo_message_add_int32(reply, parameter_index + 1); // plugin parameter index
		lo_message_add_string(reply, "Mid Low Freq"); // plugin parameter name
		int flags = 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, 0.); // lower
		lo_message_add_float(reply, 64.); // upper
		lo_message_add_string(reply, ""); // format string
		lo_message_add_int32(reply, eq_high_freq_map_size + eq_low_freq_map_size); // scale points
		for (int i = 0; i < eq_high_freq_map_size + eq_low_freq_map_size; i++) {
			lo_message_add_float(reply, (float) i); // scale points
			lo_message_add_string(reply, i < eq_low_freq_map_size ? eq_low_freq_map[i] : eq_high_freq_map[i - eq_low_freq_map_size]); // format string
		}
		lo_message_add_double(reply, (double) m_mid_low_freq_band->get_value());
	}
	if (parameter_index == 7) {
		lo_message_add_int32(reply, parameter_index + 1); // plugin parameter index
		lo_message_add_string(reply, "Mid Low Q"); // plugin parameter name
		int flags = 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, 0.); // lower
		lo_message_add_float(reply, 6.); // upper
		lo_message_add_string(reply, ""); // format string
		lo_message_add_int32(reply, 7); // scale points
		for (int i = 0; i < 7; i++) {
			char sp[16];
			lo_message_add_float(reply, (float) i); // scale points
			lo_message_add_string(reply, eq_width_text(i, sp, 16)); // format string
		}
		lo_message_add_double(reply, (double) m_mid_low_freq_width->get_value());
	}
	if (parameter_index == 8) {
		lo_message_add_int32(reply, parameter_index + 1); // plugin parameter index
		lo_message_add_string(reply, "Low gain"); // plugin parameter name
		int flags = 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, -12.); // lower
		lo_message_add_float(reply, 12.); // upper
		lo_message_add_string(reply, "%.0f dB"); // format string
		lo_message_add_int32(reply, 0); // scale points
		lo_message_add_double(reply, (double) m_low_freq_gain->get_value() - 12);
	}
	if (parameter_index == 9) {
		lo_message_add_int32(reply, parameter_index + 1); // plugin parameter index
		lo_message_add_string(reply, "Low Freq"); // plugin parameter name
		int flags = 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0x80 | 0;
		lo_message_add_int32(reply, flags); // plugin parameter flags
		lo_message_add_string(reply, "INT"); // it's data type
		lo_message_add_float(reply, 0.); // lower
		lo_message_add_float(reply, 32.); // upper
		lo_message_add_string(reply, ""); // format string
		lo_message_add_int32(reply, eq_low_freq_map_size); // scale points
		for (int i = 0; i < eq_low_freq_map_size; i++) {
			lo_message_add_float(reply, (float) i); // scale points
			lo_message_add_string(reply, eq_low_freq_map[i]); // format string
		}
		lo_message_add_double(reply, (double) m_low_freq_band->get_value());
	}
}