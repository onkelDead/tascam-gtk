/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OOscControl.h
 * Author: onkel
 *
 * Created on 28. Dezember 2024, 09:12
 */

#ifndef OOSCCONTROL_H
#define OOSCCONTROL_H

#include <gtkmm.h>

class OOscControl {
public:

    virtual ~OOscControl(){};
    
    virtual gint get_value() = 0;
    virtual void set_value(gint new_val) = 0;
    
    virtual void osc_init(const char*) = 0;
    virtual void osc_init(const char*, int) = 0;
    virtual char* get_osc_path() = 0;
    virtual int get_osc_index() = 0;
    virtual char* get_alsa_name() = 0;
    
};

#endif /* OOSCCONTROL_H */

