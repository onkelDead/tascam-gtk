/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ODspLayout.h
 * Author: onkel
 *
 * Created on January 16, 2017, 10:40 AM
 */

#ifndef ODSPLAYOUT_H
#define ODSPLAYOUT_H

#include "OTypes.h"
#include "ODial.h"
#include "OComp.h"
#include "OEq.h"
#include "OFader.h"
#include "OMeter.h"
#include "OAlsa.h"

class ODspLayout : public Gtk::VBox {
public:
    ODspLayout();
    ODspLayout(const ODspLayout& orig);
    virtual ~ODspLayout();
    
    void init(int index, OAlsa* alsa, Gtk::Window* wnd);

    void set_view_type(VIEW_TYPE i);
    
    void set_sensitive(bool val);

    void set_channel_type(CHANNEL_TYPE num_channels);
    CHANNEL_TYPE get_channel_type() {return m_channel_type;}
    
    void set_ref_index(int index, Gtk::Window* wnd);
    
    OComp m_comp;
    OEq m_eq;
private:
    Gtk::Grid m_grid;
    CHANNEL_TYPE m_channel_type;
    Gtk::VSeparator m_eq_sep;
    
    Gtk::HSeparator m_sep;

};

#endif /* ODSPLAYOUT_H */

