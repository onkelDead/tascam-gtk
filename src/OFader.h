/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OFader.h
 * Author: onkel
 *
 * Created on 28. Dezember 2024, 15:11
 */

#ifndef OFADER_H
#define OFADER_H

#include <gtkmm/widget.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/styleproperty.h>

#include "OOscControl.h"

#include "OTypes.h"

class OFader  : public Gtk::VScale, public OOscControl {
public:
    OFader();
    virtual ~OFader();
    
    gint get_value() { return (gint) Gtk::VScale::get_value(); }
    void set_value(gint val) { Gtk::VScale::set_value(val); }
    
    void set_alsa_control_name(const char*);
    
    void osc_init(const char*);
    void osc_init(const char*, int);
    char* get_osc_path();
    int get_osc_index();
    
    char* get_alsa_name() { return m_alsa_name; }
    
private:

    char m_alsa_name[64];
    char m_osc_path[64];
    int m_osc_index;
};

#endif /* OFADER_H */

