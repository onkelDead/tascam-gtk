/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OComp.h
 * Author: onkel
 *
 * Created on January 2, 2017, 12:20 PM
 */

#ifndef OCOMP_H
#define OCOMP_H

#include "ODial.h"
#include "OMeter.h"
#include "OAlsa.h"

class OComp : public Gtk::VBox {
public:
    OComp();
    virtual ~OComp();
    
    void init(int index, OAlsa* alsa);
    
    OMeter m_reduction;
    
    bool is_active() { return m_CompEnable.get_active(); }
    
private:
    Gtk::VBox m_box;

    Gtk::HBox m_tg_box;    
    ODial m_threshold;
    ODial m_gain;
    
    Gtk::HBox m_ar_box;
    ODial m_attack;
    ODial m_release;
    
    Gtk::HBox m_r_box;
    
    Gtk::VBox m_re_box;
    ODial m_ratio;
    
    Gtk::VBox l_evb;
    Gtk::ToggleButton m_CompEnable;
    
    Gtk::HBox m_red_box;

};

#endif /* OCOMP_H */

