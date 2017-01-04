/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OStripLayout.cpp
 * Author: onkel
 * 
 * Created on January 1, 2017, 2:35 PM
 */

#include <gtkmm.h>

#include "OStripLayout.h"

OStripLayout::OStripLayout() : Gtk::VBox() {
	set_size_request(90, -1);
	m_box.pack_start(m_title, false, false);
	m_title_sep.set_size_request(-1,1);
	m_box.pack_start(m_title_sep, false, false);
	
	m_box.pack_start(m_comp, false, false);
	m_comp_sep.set_size_request(-1,1);
	m_box.pack_start(m_comp_sep, false, false);
	
	m_box.pack_start(m_eq, false, false);
	m_eq_sep.set_size_request(-1,1);
	m_box.pack_start(m_eq_sep, false, false);
	
//	m_Pan.set_min(0);
	m_Pan.set_max(254);
	m_Pan.set_default(127);
	m_Pan.set_label("L Pan R");
	m_Pan.set_knob_background_color(1., .8, .3, 1.);
	m_panbox.pack_start(m_Pan, false, false );
	
	Gdk::Color mute_color;
	Gdk::Color solo_color;
	Gdk::Color mute_color_a;
	Gdk::Color solo_color_a;
	solo_color.set_rgb_p(.8,1,.8);
	solo_color_a.set_rgb_p(.7,1,.7);
	mute_color_a.set_rgb_p(1,1,.7);
	mute_color.set_rgb_p(1,1,.8);
	m_MuteEnable.modify_bg(Gtk::STATE_PRELIGHT, mute_color);
	m_MuteEnable.modify_bg(Gtk::STATE_ACTIVE, mute_color_a);
	m_SoloEnable.modify_bg(Gtk::STATE_PRELIGHT, solo_color);
	m_SoloEnable.modify_bg(Gtk::STATE_ACTIVE, solo_color_a);

	m_MuteEnable.set_label("Mute");
	m_MuteEnable.get_child()->modify_font(Pango::FontDescription("System 8"));
	m_SoloEnable.set_label("Solo");
	m_SoloEnable.get_child()->modify_font(Pango::FontDescription("System 8"));

	Gdk::Color phase_color;
	phase_color.set_rgb_p(.8,1,1);
	m_PhaseEnable.modify_bg(Gtk::STATE_PRELIGHT, phase_color);
	Gdk::Color phase_color_a;
	phase_color_a.set_rgb_p(.7,1,1);
	m_PhaseEnable.modify_bg(Gtk::STATE_ACTIVE, phase_color_a);
	m_PhaseEnable.set_label("Phase");
	m_PhaseEnable.get_child()->modify_font(Pango::FontDescription("System 8"));
	
	m_pan_button_box.pack_start(m_MuteEnable, false, false);
	m_pan_button_box.pack_start(m_SoloEnable, false, false);
	m_pan_button_box.pack_start(m_PhaseEnable, false, false);
	m_panbox.pack_start(m_pan_button_box, false, false);
	m_box.pack_start(m_panbox, false, false);
	
	m_meter.set_size_request(10, 160);
	m_fader_box.pack_start(m_meter, true, false);

	m_fader.set_range(0, 133);
	m_fader.set_inverted (true);
	m_fader.set_size_request(-1, 160);
	m_fader.set_draw_value(false);
	m_fader.set_tooltip_text("channel fader");
	m_fader_box.pack_start(m_fader, true, true);

	m_box.pack_start(m_fader_box);
	
	m_hbox.pack_start(m_box);
	m_sep.set_size_request(1, -1);
	m_hbox.pack_start(m_sep);
	
	
	add(m_hbox);
}

OStripLayout::~OStripLayout() {
}

void OStripLayout::init(int index, OAlsa* alsa) {
	char l_title[64];
	int val;

	snprintf(l_title, sizeof(l_title), "Ch %d", index + 1);
	m_title.set_text(l_title);
	
	val = alsa->getInteger(CTL_NAME_FADER, index);
	m_fader.set_value(val);
	snprintf(l_title, sizeof(l_title), "%d", val);
	m_fader.set_tooltip_text(l_title);
	m_fader.signal_value_changed().connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_range_control_changed), index, CTL_NAME_FADER, &m_fader));
			
	m_Pan.set_value(alsa->getInteger(CTL_NAME_PAN, index));
	m_Pan.signal_value_changed.connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_PAN, &m_Pan));
			
	m_MuteEnable.set_active(alsa->getBoolean(CTL_NAME_MUTE, index));
	m_MuteEnable.signal_toggled().connect(sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_toggle_button_control_changed), index, CTL_NAME_MUTE, &m_MuteEnable));

	m_SoloEnable.set_active(alsa->getBoolean(CTL_NAME_SOLO, index));
	m_SoloEnable.signal_toggled().connect(sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_toggle_button_control_changed), index, CTL_NAME_SOLO, &m_SoloEnable));

	m_PhaseEnable.set_active(alsa->getBoolean(CTL_NAME_PHASE, index));
	m_PhaseEnable.signal_toggled().connect(sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_toggle_button_control_changed), index, CTL_NAME_PHASE, &m_PhaseEnable));

	m_comp.init(index, alsa);
	m_eq.init(index, alsa);
}

