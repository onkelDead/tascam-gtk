/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OMaster.h
 * Author: onkel
 *
 * Created on January 3, 2017, 12:27 PM
 */

#ifndef OMASTER_H
#define OMASTER_H

#include "ODial.h"
#include "OAlsa.h"
#include "OMeter.h"

class OMaster : public Gtk::VBox  {
public:
    OMaster();
    virtual ~OMaster();
    
    void init(int index, OAlsa* alsa);    
    
    void reset(OAlsa* alsa);
    
    OMeter m_meter_left;
    OMeter m_meter_right;
    
    Gtk::VScale m_fader;
    Gtk::ToggleButton m_mute;
    Gtk::ToggleButton m_true_bypass;
    Gtk::ToggleButton m_comp_to_stereo;
    Gtk::ComboBoxText m_route[8];
    
private:
    
    Gtk::VBox m_route_box;
    
    Gtk::HBox m_fader_box;
    
};

#endif /* OMASTER_H */

