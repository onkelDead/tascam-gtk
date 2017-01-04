/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OMaster.cpp
 * Author: onkel
 * 
 * Created on January 3, 2017, 12:27 PM
 */

#include <gtkmm.h>

#include "OMaster.h"

OMaster::OMaster() : Gtk::VBox()  {
	set_size_request(120, -1);
	Gdk::Color color;
	color.set_rgb_p(.25,.25,.25);
	Gdk::Color fcolor;
	fcolor.set_rgb_p(.85,.85,.85);
	
	for( int i = 0; i < 8 ; i++) {
		
        m_route[i].append("Master Left");
        m_route[i].append("Master Right");
        for( int j = 0; j < 8; j++) {
			char entry[24];
			snprintf(entry, 24, "Output %d", j+1);
            m_route[i].append(entry);
        }		
		m_route[i].modify_bg(Gtk::STATE_NORMAL, color);
		m_route[i].get_child()->modify_bg(Gtk::STATE_NORMAL, color);
		m_route_box.pack_start(m_route[i], true, true);
	}
	
	pack_start(m_route_box, false, false);
	
	m_true_bypass.set_label("Mixer True\nBypass");
	m_true_bypass.set_size_request(-1, 48);
	pack_start(m_true_bypass, false, true);
	m_comp_to_stereo.set_label("Computer out\nto Stereo BUS");
	m_comp_to_stereo.set_size_request(-1, 48);
	pack_start(m_comp_to_stereo, false, true);
	
	m_meter_left.set_size_request(20, 160);
	m_meter_right.set_size_request(20, 160);
	m_fader_box.pack_start(m_meter_left, true, false);
	m_fader_box.pack_start(m_meter_right, true, false);
		
	m_fader.set_range(0, 133);
	m_fader.set_inverted (true);
	m_fader.set_size_request(-1, 160);
	m_fader.set_draw_value(false);
	m_fader.set_tooltip_text("left/right master fader");
	
	m_fader_box.pack_start(m_fader);
	pack_start(m_fader_box);

	modify_bg(Gtk::STATE_NORMAL, color);
	
	fcolor.set_rgb_p(.85,.85,.85);
	color.set_rgb_p(.95,.65,.65);
	m_true_bypass.modify_bg(Gtk::STATE_NORMAL, color);	
	m_comp_to_stereo.modify_bg(Gtk::STATE_NORMAL, color);	
	m_true_bypass.modify_bg(Gtk::STATE_PRELIGHT, color);	
	m_comp_to_stereo.modify_bg(Gtk::STATE_PRELIGHT, color);	
	color.set_rgb_p(.25,.25,.25);
	m_true_bypass.modify_bg(Gtk::STATE_ACTIVE, color);	
	m_comp_to_stereo.modify_bg(Gtk::STATE_ACTIVE, color);	
	m_true_bypass.get_child()->modify_fg(Gtk::STATE_NORMAL, color);
	m_true_bypass.get_child()->modify_fg(Gtk::STATE_PRELIGHT, color);
	m_comp_to_stereo.get_child()->modify_fg(Gtk::STATE_NORMAL, color);
	m_comp_to_stereo.get_child()->modify_fg(Gtk::STATE_PRELIGHT, color);
	m_true_bypass.get_child()->modify_fg(Gtk::STATE_ACTIVE, fcolor);
	m_comp_to_stereo.get_child()->modify_fg(Gtk::STATE_ACTIVE, fcolor);
//	modify_fg(Gtk::STATE_NORMAL, color);	
}

OMaster::~OMaster() {
}

void OMaster::init(int index, OAlsa* alsa) {
	int val;
	
	val = alsa->getInteger(CTL_MASTER, index);
	m_fader.set_value(val);
	m_fader.signal_value_changed().connect( sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_range_control_changed), index, CTL_MASTER, &m_fader));	
	
	
    for(int ri = 0; ri < 8; ri++ ) {
        val = alsa->getInteger(CTL_ROUTE, ri);
		m_route[ri].set_active(val);
		m_route[ri].signal_changed().connect(sigc::bind<>(sigc::mem_fun (alsa, &OAlsa::on_combo_control_changed), ri, CTL_ROUTE, &m_route[ri]));	
    }
	
}