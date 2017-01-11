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
#include <libxml++-2.6/libxml++/libxml++.h>
#include <libxml++-2.6/libxml++/parsers/textreader.h>

#include "OComp.h"

#define CREAD_LIGHT 1., .8, .8, 1.
#define CREAD_NORMAL 1., .6, .6, 1.


static const char *cp_ration_map[] = { "1.0:1", "1.1:1", "1.3:1", "1.5:1", "1.7:1", "2.0:1", "2.5:1", "3.0:1", "3.5:1", "4.0:1", "5.0:1", "6.0:1", "8.0:1", "16.0:1", "inf:1" };

char* cp_threshold_text(int val, char* buf, size_t buf_size) {
    snprintf(buf, buf_size, "%ddb", val - 32);
	return buf;
}

char* cp_gain_text(int val, char* buf, size_t buf_size) {
    snprintf(buf, buf_size, val > 0 ? "+%ddb" : "%ddb", val );
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
	m_CompEnable.set_size_request(40, -1);
	m_CompEnable.set_label("Comp");
	m_CompEnable.set_name("comp-button");
	
	Gdk::Color color_b;
	color_b.set_rgb_p(.25,.25,.25);		
	Gdk::Color color;
	color.set_rgb_p(1., .8, .8);
	Gdk::Color color_a;
	color_a.set_rgb_p(1., .6, .6);

	m_threshold.set_params(0,32,32,1);
	m_threshold.set_label("Thresh");
	m_threshold.set_value_callback(cp_threshold_text);
	m_threshold.set_knob_background_color(CREAD_NORMAL);
	m_tg_box.pack_start(m_threshold, true, false);
	
	m_gain.set_params(0,20,0,1);
	m_gain.set_label("Gain");
	m_gain.set_value_callback(cp_gain_text);
	m_gain.set_knob_background_color(CREAD_NORMAL);
	m_tg_box.pack_start(m_gain, true, false);
	
	m_attack.set_params(0,198,0,5);
	m_attack.set_label("Attack");
	m_attack.set_value_callback(cp_attack_text);
	m_attack.set_knob_background_color(CREAD_LIGHT);
	m_ar_box.pack_start(m_attack, true, false);
	
	m_release.set_params(0,99,0,1);
	m_release.set_label("Release");
	m_release.set_value_callback(cp_release_text);
	m_release.set_knob_background_color(CREAD_LIGHT);
	m_ar_box.pack_start(m_release, true, false);
	
	m_ratio.set_params(0,14,0,1);
	m_ratio.set_label("Ratio");
	m_ratio.set_map(cp_ration_map);
	m_ratio.set_knob_background_color(CREAD_NORMAL);
	m_re_box.pack_start(m_ratio, true, false);

	
	l_evb.pack_start(m_CompEnable, true, false);
	
	m_re_box.pack_start(l_evb, true, false);
	m_r_box.pack_start(m_re_box, true, false);
	
	m_reduction.setLevel(32768);
	m_reduction.set_size_request(10,-1);
	m_reduction.set_level_direction(1);
	m_reduction.set_level_color(1, .6, .6, 1);
	m_red_box.pack_start(m_reduction, true, false);
	m_r_box.pack_start(m_red_box);
	
	m_box.pack_start(m_tg_box);
	m_box.pack_start(m_ar_box);
	m_box.pack_start(m_r_box);
	
	add(m_box);
}

OComp::~OComp() {
}

void OComp::init(int index, OAlsa* alsa) {

	m_CompEnable.set_active(alsa->getBoolean(CTL_NAME_CP_ENABLE, index));
	m_CompEnable.signal_toggled().connect(sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_toggle_button_control_changed), index, CTL_NAME_CP_ENABLE, &m_CompEnable));

	m_threshold.set_value(alsa->getInteger(CTL_NAME_CP_THRESHOLD, index));
	m_threshold.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_CP_THRESHOLD, &m_threshold));
	
	m_gain.set_value(alsa->getInteger(CTL_NAME_CP_GAIN, index));
	m_gain.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_CP_GAIN, &m_gain));
	
	m_attack.set_value(alsa->getInteger(CTL_NAME_CP_ATTACK, index));
	m_attack.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_CP_ATTACK, &m_attack));
	
	m_release.set_value(alsa->getInteger(CTL_NAME_CP_RELEASE, index));
	m_release.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_CP_RELEASE, &m_release));
	
	m_ratio.set_value(alsa->getInteger(CTL_NAME_CP_RATIO, index));
	m_ratio.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_CP_RATIO, &m_ratio));
	
}

void OComp::reset(OAlsa* alsa, int index) {
	
	alsa->setBoolean(CTL_NAME_CP_ENABLE, 0, 0);
	m_CompEnable.set_active(alsa->getBoolean(CTL_NAME_CP_ENABLE, 0));
	usleep(RESET_VALUE_DELAY);

	m_threshold.reset();
	usleep(RESET_VALUE_DELAY);
	
	m_gain.reset();
	usleep(RESET_VALUE_DELAY);
	
	m_attack.reset();
	usleep(RESET_VALUE_DELAY);
	
	m_release.reset();
	usleep(RESET_VALUE_DELAY);

	m_ratio.reset();
	usleep(RESET_VALUE_DELAY);
	
}

void OComp::save_values(FILE* file) {
	
	fprintf(file, "\t\t\t<enable>");
	fprintf(file, "%d", (int) m_CompEnable.get_active());
	fprintf(file, "</enable>\n");
	
	fprintf(file, "\t\t\t<threshold>");
	fprintf(file, "%d", (int) m_threshold.get_value());
	fprintf(file, "</threshold>\n");
	
	fprintf(file, "\t\t\t<gain>");
	fprintf(file, "%d", (int) m_gain.get_value());
	fprintf(file, "</gain>\n");
	
	fprintf(file, "\t\t\t<attack>");
	fprintf(file, "%d", (int) m_attack.get_value());
	fprintf(file, "</attack>\n");
	
	fprintf(file, "\t\t\t<release>");
	fprintf(file, "%d", (int) m_release.get_value());
	fprintf(file, "</release>\n");
	
	fprintf(file, "\t\t\t<ratio>");
	fprintf(file, "%d", (int) m_ratio.get_value());
	fprintf(file, "</ratio>\n");
	
}

void OComp::load_values(Glib::ustring xml) {
	
	try {
		xmlpp::TextReader reader((const unsigned char*)xml.c_str(), xml.size());
		
		while (reader.read()) {
			if( !strcmp(reader.get_name().c_str(), "enable") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_CompEnable.set_active(atoi(reader.get_value().c_str()) == 1);
				m_CompEnable.toggled();
				usleep(RESET_VALUE_DELAY);
			}				
			if( !strcmp(reader.get_name().c_str(), "threshold") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_threshold.set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if( !strcmp(reader.get_name().c_str(), "gain") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_gain.set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if( !strcmp(reader.get_name().c_str(), "attack") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_attack.set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if( !strcmp(reader.get_name().c_str(), "release") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_release.set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if( !strcmp(reader.get_name().c_str(), "ratio") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_ratio.set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
		}		
	} catch (const std::exception& e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return;
	}		
}
		