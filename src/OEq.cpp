/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OEq.cpp
 * Author: onkel
 * 
 * Created on January 2, 2017, 12:58 PM
 */

#include <gtkmm.h>
#include <stdbool.h>
#include <gtkmm-3.0/gtkmm/box.h>

#include "OEq.h"
#include "ODial.h"

static const char *eq_low_freq_map[] = { "32", "40", "50", "60", "70", "80", "90", "100", "125", "150", "175", "200", "225", "250",
                                   "300", "350", "400", "450", "500", "600", "700", "800", "850", "900", "950", "1.0k",
                                   "1.1k", "1.2k", "1.3k", "1.4k", "1.5k", "1.6k"};
static const int eq_low_freq_map_size = 32;


static const char *eq_high_freq_map[] = { "1.7k", "1.8k", "1.9k", "2.0k", "2.2k", "2.4k", "2.6k", "2.8k", "3.0k", "3.2k", "3.4k", "3.6k", "3.8k", "4.0k",
                                            "4.5k", "5.0k", "5.5k", "6.0k", "6.5k", "7.0k", "7.5k", "8.0k", "8.5k",
                                            "9.0k", "10k", "11k", "12k", "13k", "14k", "15k", "16k", "17k", "18k" };

char* eq_width_text(int val, char* buf) {
    int a = (1 << val);
    float b = a;
    float c =  b / 4;
    sprintf(buf, "Q%2.2f", c );
	return buf;
}

char* eq_level_text(int val, char* buf) {
	int v = val - 12;
    sprintf(buf, v > 0 ? "+%ddB" : "%ddB", v );
	return buf;
}

char* eq_high_freq_text(int val, char* buf) {
    sprintf(buf, "%sHz", eq_high_freq_map[val] );
	return buf;
}

char* eq_lowhigh_freq_text(int val, char* buf) {

    if( val < eq_low_freq_map_size )
        sprintf(buf, "%sHz", eq_low_freq_map[val] );
    else
        sprintf(buf, "%sHz", eq_high_freq_map[val - eq_low_freq_map_size] );
	return buf;
}

char* eq_low_freq_text(int val, char* buf) {
    sprintf(buf, "%sHz", eq_low_freq_map[val] );
	return buf;
}

OEq::OEq() : Gtk::VBox() {
	set_size_request(80, -1);
	
	m_EqEnable.set_label("EQ");
	
	m_high_freq_gain.set_label("High");
	m_high_freq_gain.set_value_callback(eq_level_text);
	m_high_freq_gain.set_params(0,24,12,1);
	m_high_box.pack_start(m_high_freq_gain, false, false);

	m_high_freq_band.set_label("Freq");
	m_high_freq_band.set_value_callback(eq_high_freq_text);
	m_high_freq_band.set_params(0,31,15,1);
	m_high_box.pack_start(m_high_freq_band, false, false);
	
	m_mid_high_freq_gain.set_label("Mid H");
	m_mid_high_freq_gain.set_params(0,24,12,1);
	m_mid_high_freq_gain.set_value_callback(eq_high_freq_text);
	m_mid_high_box.pack_start(m_mid_high_freq_gain, false, false);
	
	m_mid_high_freq_band.set_label("Freq");
	m_mid_high_freq_band.set_params(0,31,27,1);
	m_mid_high_freq_band.set_value_callback(eq_lowhigh_freq_text);
	m_mid_high_box.pack_start(m_mid_high_freq_band, false, false);
	
	m_mid_high_freq_width.set_label("Width");
	m_mid_high_freq_width.set_value_callback(eq_width_text);
	m_mid_high_freq_width.set_params(0,6,2,1);
	m_mid_high_box1.pack_start(m_mid_high_freq_width, false, false);
	
	m_mid_low_freq_gain.set_label("Mid L");
	m_mid_low_freq_gain.set_params(0,24,12,1);
	m_mid_low_freq_gain.set_value_callback(eq_level_text);
	m_mid_low_box.pack_start(m_mid_low_freq_gain, false, false);

	m_mid_low_freq_band.set_label("Freq");
	m_mid_low_freq_band.set_params(0,31,14,1);
	m_mid_low_freq_band.set_value_callback(eq_lowhigh_freq_text);
	m_mid_low_box.pack_start(m_mid_low_freq_band, false, false);
	
	m_mid_low_freq_width.set_label("Width");
	m_mid_low_freq_width.set_value_callback(eq_width_text);
	m_mid_low_freq_width.set_params(0,6,2,1);
	m_mid_low_box1.pack_start(m_mid_low_freq_width, false, false);
	m_mid_low_box1.pack_start(m_EqEnable, true, true);
	
	m_low_freq_gain.set_label("Low");
	m_low_freq_gain.set_params(0,24,12,1);
	m_low_freq_gain.set_value_callback(eq_level_text);
	m_low_box.pack_start(m_low_freq_gain, false, false);

	m_low_freq_band.set_label("Freq");
	m_low_freq_band.set_params(0,31,5,1);
	m_low_freq_band.set_value_callback(eq_low_freq_text);
	m_low_box.pack_start(m_low_freq_band, false, false);
	
	m_box.pack_start(m_high_box, false, false);
	m_box.pack_start(m_mid_high_box, false, false);
	m_box.pack_start(m_mid_high_box1, false, false);
	m_box.pack_start(m_mid_low_box, false, false);
	m_box.pack_start(m_mid_low_box1, false, false);
	m_box.pack_start(m_low_box, false, false);
	
	add(m_box);
}

OEq::~OEq() {
}

void OEq::init(int index, OAlsa* alsa) {

	m_EqEnable.set_active(alsa->getBoolean(CTL_NAME_EQ_ENABLE, index));
	m_EqEnable.signal_toggled().connect(sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_toggle_button_control_changed), index, CTL_NAME_EQ_ENABLE, &m_EqEnable));

	m_high_freq_gain.set_value(alsa->getInteger(CTL_NAME_EQ_HIGH_LEVEL, index));
	m_high_freq_gain.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_EQ_HIGH_LEVEL, &m_high_freq_gain));
	
	m_high_freq_band.set_value(alsa->getInteger(CTL_NAME_EQ_HIGH_FREQ, index));
	m_high_freq_band.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_EQ_HIGH_FREQ, &m_high_freq_band));
	
	m_mid_high_freq_gain.set_value(alsa->getInteger(CTL_NAME_EQ_MIDHIGH_LEVEL, index));
	m_mid_high_freq_gain.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_EQ_MIDHIGH_LEVEL, &m_mid_high_freq_gain));
	
	m_mid_high_freq_band.set_value(alsa->getInteger(CTL_NAME_EQ_MIDHIGH_FREQ, index));
	m_mid_high_freq_band.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_EQ_MIDHIGH_FREQ, &m_mid_high_freq_band));

	m_mid_high_freq_width.set_value(alsa->getInteger(CTL_NAME_EQ_MIDHIGHWIDTH_FREQ, index));
	m_mid_high_freq_width.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_EQ_MIDHIGHWIDTH_FREQ, &m_mid_high_freq_width));

	m_mid_low_freq_gain.set_value(alsa->getInteger(CTL_NAME_EQ_MIDLOW_LEVEL, index));
	m_mid_low_freq_gain.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_EQ_MIDLOW_LEVEL, &m_mid_low_freq_gain));

	m_mid_low_freq_band.set_value(alsa->getInteger(CTL_NAME_EQ_MIDLOW_FREQ, index));
	m_mid_low_freq_band.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_EQ_MIDLOW_FREQ, &m_mid_low_freq_band));

	m_mid_low_freq_width.set_value(alsa->getInteger(CTL_NAME_EQ_MIDLOWWIDTH_FREQ, index));
	m_mid_low_freq_width.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_EQ_MIDLOWWIDTH_FREQ, &m_mid_low_freq_width));

	m_low_freq_gain.set_value(alsa->getInteger(CTL_NAME_EQ_LOW_LEVEL, index));
	m_low_freq_gain.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_EQ_LOW_LEVEL, &m_low_freq_gain));

	m_low_freq_band.set_value(alsa->getInteger(CTL_NAME_EQ_LOW_FREQ, index));
	m_low_freq_band.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_EQ_LOW_FREQ, &m_low_freq_band));

}