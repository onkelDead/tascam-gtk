/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OComp.cpp
 * Author: onkel
 * 
 * Created on January 2, 2017, 12:20 PM
 */

#include <gtkmm.h>
#include <stdbool.h>

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
	set_size_request(90, -1);
//	m_CompEnable.set_size_request(40, 32);
	m_CompEnable.set_label("Comp");
	m_CompEnable.get_child()->modify_font(Pango::FontDescription("System 8"));
	
	Gdk::Color color_b;
	color_b.set_rgb_p(.25,.25,.25);		
	Gdk::Color color;
	color.set_rgb_p(1., .8, .8);
	m_CompEnable.modify_bg(Gtk::STATE_PRELIGHT, color);
	Gdk::Color color_a;
	color_a.set_rgb_p(1., .6, .6);
	m_CompEnable.modify_bg(Gtk::STATE_ACTIVE, color_a);
	m_CompEnable.modify_bg(Gtk::STATE_NORMAL, color_b);
	m_CompEnable.get_child()->modify_fg(Gtk::STATE_NORMAL, color);

	m_threshold.set_params(0,32,32,1);
	m_threshold.set_label("Thresh");
	m_threshold.set_value_callback(cp_threshold_text);
	m_threshold.set_knob_background_color(CREAD_NORMAL);
	m_tg_box.pack_start(m_threshold, false, false);
	
	m_gain.set_params(0,20,0,1);
	m_gain.set_label("Gain");
	m_gain.set_value_callback(cp_gain_text);
	m_gain.set_knob_background_color(CREAD_NORMAL);
	m_tg_box.pack_start(m_gain, false, false);
	
	m_attack.set_params(0,198,0,5);
	m_attack.set_label("Attack");
	m_attack.set_value_callback(cp_attack_text);
	m_attack.set_knob_background_color(CREAD_LIGHT);
	m_ar_box.pack_start(m_attack, false, false);
	
	m_release.set_params(0,99,0,1);
	m_release.set_label("Release");
	m_release.set_value_callback(cp_release_text);
	m_release.set_knob_background_color(CREAD_LIGHT);
	m_ar_box.pack_start(m_release, false, false);
	
	m_ratio.set_params(0,14,0,1);
	m_ratio.set_label("Ratio");
	m_ratio.set_map(cp_ration_map);
	m_ratio.set_knob_background_color(CREAD_NORMAL);
	m_re_box.pack_start(m_ratio, false, false);

	
	l_evb.pack_start(m_CompEnable, true, false);
	
	m_re_box.pack_start(l_evb, false, false);
	
	m_box.pack_start(m_tg_box, false, false);
	m_box.pack_start(m_ar_box, false, false);
	m_box.pack_start(m_re_box, false, false);
	
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