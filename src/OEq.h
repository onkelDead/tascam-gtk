/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OEq.h
 * Author: onkel
 *
 * Created on January 2, 2017, 12:58 PM
 */

#ifndef OEQ_H
#define OEQ_H

#include "ODial.h"
#include "OAlsa.h"

class OEq : public Gtk::VBox {
public:
    OEq();
    virtual ~OEq();
    
    void init(int index, OAlsa* alsa);
    
private:
    Gtk::VBox m_box;

    Gtk::VBox l_eeb;
    Gtk::ToggleButton m_EqEnable;

    Gtk::HBox m_high_box;
    ODial m_high_freq_gain;
    ODial m_high_freq_band;
    
    Gtk::HBox m_mid_high_box;
    ODial m_mid_high_freq_gain;
    ODial m_mid_high_freq_band;
    Gtk::HBox m_mid_high_box1;
    ODial m_mid_high_freq_width;
    
    Gtk::HBox m_mid_low_box;
    ODial m_mid_low_freq_gain;
    ODial m_mid_low_freq_band;
    Gtk::HBox m_mid_low_box1;
    ODial m_mid_low_freq_width;
    
    Gtk::HBox m_low_box;
    ODial m_low_freq_gain;
    ODial m_low_freq_band;
    
};

#endif /* OEQ_H */

