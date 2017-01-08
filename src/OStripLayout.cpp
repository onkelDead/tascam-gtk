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

	Gdk::Color tcolor;
	tcolor.set_rgb_p(.8, .8, .8);
	Gdk::Color dbcolor;
	dbcolor.set_rgb_p(0, .9, 0);

	// strip title	
	m_title.modify_fg(Gtk::STATE_NORMAL, tcolor);
	m_box.pack_start(m_title, false, false);

	m_title_sep.set_size_request(-1, 1);
	m_box.pack_start(m_title_sep, false, true, 0);

	// strip compressor	
	m_box.pack_start(m_comp, false, false);
	m_comp_sep.set_size_request(-1, 1);
	m_box.pack_start(m_comp_sep, false, false);

	// strip EQ	
	m_box.pack_start(m_eq, false, false);
	m_eq_sep.set_size_request(-1, 1);
	m_box.pack_start(m_eq_sep, false, false);

	Gdk::Color mute_color;
	Gdk::Color solo_color;
	Gdk::Color mute_color_a;
	Gdk::Color solo_color_a;
	solo_color.set_rgb_p(.8, 1, .8);
	solo_color_a.set_rgb_p(.7, 1, .7);
	mute_color_a.set_rgb_p(1, 1, .7);
	mute_color.set_rgb_p(.8, .8, .6);
	m_MuteEnable.modify_bg(Gtk::STATE_PRELIGHT, mute_color);
	m_MuteEnable.modify_bg(Gtk::STATE_ACTIVE, mute_color_a);
	m_SoloEnable.modify_bg(Gtk::STATE_PRELIGHT, solo_color);
	m_SoloEnable.modify_bg(Gtk::STATE_ACTIVE, solo_color_a);

	Gdk::Color color;
	color.set_rgb_p(.25, .25, .25);
	m_MuteEnable.set_label("Mute");
	m_MuteEnable.get_child()->modify_font(Pango::FontDescription("System 8"));
	m_SoloEnable.set_label("Solo");
	m_MuteEnable.modify_bg(Gtk::STATE_NORMAL, color);
	m_MuteEnable.get_child()->modify_fg(Gtk::STATE_NORMAL, mute_color_a);
	m_SoloEnable.modify_bg(Gtk::STATE_NORMAL, color);
	m_SoloEnable.get_child()->modify_font(Pango::FontDescription("System 8"));
	m_SoloEnable.get_child()->modify_fg(Gtk::STATE_NORMAL, solo_color_a);

	Gdk::Color phase_color;
	phase_color.set_rgb_p(.8, 1, 1);
	m_PhaseEnable.set_label("Phase");
	m_PhaseEnable.modify_bg(Gtk::STATE_NORMAL, color);
	m_PhaseEnable.modify_bg(Gtk::STATE_PRELIGHT, phase_color);
	Gdk::Color phase_color_a;
	phase_color_a.set_rgb_p(.7, 1, 1);
	m_PhaseEnable.modify_bg(Gtk::STATE_ACTIVE, phase_color_a);
	m_PhaseEnable.get_child()->modify_fg(Gtk::STATE_NORMAL, phase_color);
	m_PhaseEnable.get_child()->modify_font(Pango::FontDescription("System 8"));

	m_pan_button_box.pack_start(m_MuteEnable, false, false);
	m_pan_button_box.pack_start(m_SoloEnable, false, false);
	m_pan_button_box.pack_start(m_PhaseEnable, false, false);

	m_Pan.set_params(0, 254, 127, 5);
	m_Pan.set_label("L Pan R");
	m_Pan.set_knob_background_color(1., .8, .3, 1.);

	m_dB.modify_font(Pango::FontDescription("System 6"));
	m_dB.modify_fg(Gtk::STATE_NORMAL, dbcolor);


	m_meter.set_size_request(10, 160);
	m_meter.set_level_color(0, .7, 0, 1);
	m_fader.set_range(0, 133);
	m_fader.set_inverted(true);
	m_fader.set_size_request(-1, 160);
	m_fader.set_draw_value(false);
	m_fader.set_increments(1, 5);
	m_fader.add_mark(133, Gtk::PositionType::POS_RIGHT, "+6 dB");
	m_fader.add_mark(123, Gtk::PositionType::POS_RIGHT, "+3 dB");
	m_fader.add_mark(113, Gtk::PositionType::POS_RIGHT, "0 dB");
	m_fader.add_mark(89, Gtk::PositionType::POS_RIGHT, "-10 dB");
	m_fader.add_mark(73, Gtk::PositionType::POS_RIGHT, "-20 dB");
	m_fader.add_mark(50, Gtk::PositionType::POS_RIGHT, "-40 dB");
	m_fader.add_mark(34, Gtk::PositionType::POS_RIGHT, "-60 dB");
	m_fader.add_mark(16, Gtk::PositionType::POS_RIGHT, "-90 dB");
	m_fader.add_mark(0, Gtk::PositionType::POS_RIGHT, "-inf dB");
	m_fader.modify_font(Pango::FontDescription("System 6"));
	m_fader.modify_fg(Gtk::STATE_NORMAL, tcolor);
	m_fader.set_tooltip_text("channel fader");

	m_panbox.pack_start(m_pan_button_box, false, false);
	m_pvbox.pack_start(m_Pan, false, false);
	m_pvbox.pack_start(m_dB, false, false);
	m_panbox.pack_start(m_pvbox, false, false);
	m_box.pack_start(m_panbox, false, false);
	m_fader_box.pack_start(m_fader, true, false);
	m_fader_box.pack_start(m_meter, true, false);

	m_box.pack_start(m_fader_box);

	m_hbox.pack_start(m_box);
	m_sep.set_size_request(1, -1);
	m_hbox.pack_start(m_sep, false, false);


	add(m_hbox);
}

OStripLayout::~OStripLayout() {
}

void OStripLayout::init(int index, OAlsa* alsa) {
	char l_title[64];
	int val;

	snprintf(l_title, sizeof (l_title), "Ch %d", index + 1);
	m_title.set_text(l_title);

	val = alsa->getInteger(CTL_NAME_FADER, index);
	m_fader.set_value(alsa->dBToSlider(val));

	snprintf(l_title, sizeof (l_title), "%d dB", val - 127);
	m_fader.set_tooltip_text(l_title);
	m_dB.set_label(l_title);

	m_fader.signal_value_changed().connect(sigc::bind<>(sigc::mem_fun(alsa, &OAlsa::on_range_control_changed), index, CTL_NAME_FADER, &m_fader, &m_dB));

	m_Pan.set_value(alsa->getInteger(CTL_NAME_PAN, index));
	m_Pan.signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(alsa, &OAlsa::on_dial_control_changed), index, CTL_NAME_PAN, &m_Pan));

	m_MuteEnable.set_active(alsa->getBoolean(CTL_NAME_MUTE, index));
	m_MuteEnable.signal_toggled().connect(sigc::bind<>(sigc::mem_fun(alsa, &OAlsa::on_toggle_button_control_changed), index, CTL_NAME_MUTE, &m_MuteEnable));

	m_SoloEnable.set_active(alsa->getBoolean(CTL_NAME_SOLO, index));
	m_SoloEnable.signal_toggled().connect(sigc::bind<>(sigc::mem_fun(alsa, &OAlsa::on_toggle_button_control_changed), index, CTL_NAME_SOLO, &m_SoloEnable));

	m_PhaseEnable.set_active(alsa->getBoolean(CTL_NAME_PHASE, index));
	m_PhaseEnable.signal_toggled().connect(sigc::bind<>(sigc::mem_fun(alsa, &OAlsa::on_toggle_button_control_changed), index, CTL_NAME_PHASE, &m_PhaseEnable));

	m_comp.init(index, alsa);
	m_eq.init(index, alsa);
}

void OStripLayout::reset(OAlsa* alsa, int index) {


	alsa->setInteger(CTL_NAME_FADER, index, 127);
	m_fader.set_value(alsa->dBToSlider(127) + 1);
	usleep(RESET_VALUE_DELAY);

	m_Pan.reset();
	usleep(RESET_VALUE_DELAY);

	alsa->setBoolean(CTL_NAME_MUTE, 0, 0);
	m_MuteEnable.set_active(alsa->getBoolean(CTL_NAME_MUTE, 0));
	usleep(RESET_VALUE_DELAY);

	alsa->setBoolean(CTL_NAME_SOLO, 0, 0);
	m_SoloEnable.set_active(alsa->getBoolean(CTL_NAME_SOLO, 0));
	usleep(RESET_VALUE_DELAY);

	alsa->setBoolean(CTL_NAME_PHASE, 0, 0);
	m_PhaseEnable.set_active(alsa->getBoolean(CTL_NAME_PHASE, 0));
	usleep(RESET_VALUE_DELAY);

	m_eq.reset(alsa, index);
	m_comp.reset(alsa, index);

}