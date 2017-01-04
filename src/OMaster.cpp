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
	
	for( int i = 0; i < 8 ; i++) {
		
        m_route[i].append("Master Left");
        m_route[i].append("Master Right");
        for( int j = 0; j < 8; j++) {
			char entry[24];
			snprintf(entry, 24, "Output %d", j+1);
            m_route[i].append(entry);
        }		
		
		m_route_box.pack_start(m_route[i], true, true);
	}
	
	pack_start(m_route_box, false, false);
	
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