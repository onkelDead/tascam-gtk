/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OFader.h
 * Author: onkel
 *
 * Created on January 15, 2017, 10:17 AM
 */

#ifndef OFADER_H
#define OFADER_H

#include <iostream>

#include <lo/lo.h>

#include "OMeter.h"
#include "ODial.h"
#include "OAlsa.h"

class OFader : public Gtk::VBox {
public:
    OFader();
    OFader(const OFader& orig);
    virtual ~OFader();

    void init(int index, OAlsa* alsa, Gtk::Window* wnd);
    void pack(int layout);
    void unpack();
    
    void reset(OAlsa* alsa, int index);
 
    void save_values(FILE* file);    
    void load_values(Glib::ustring xml);    
    
    Gtk::ToggleButton* m_MuteEnable;
    Gtk::ToggleButton* m_SoloEnable;
    Gtk::ToggleButton* m_PhaseEnable[2];    
    Gtk::VScale* m_fader;
    ODial* m_Pan[2];
    
    OMeter m_meter[2];
    Gtk::Label m_dB;

    int m_pack;
    
private:
    Gtk::Grid m_grid;
    
};

#endif /* OFADER_H */
