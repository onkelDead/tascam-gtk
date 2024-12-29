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
    OOscControl();
    virtual ~OOscControl();
    
    virtual gint get_value() = 0;
    virtual void set_value(gint new_val) = 0;
    
    virtual void osc_init(const char*);
    virtual void osc_init(const char*, int);
    virtual char* get_osc_path();
    virtual int get_osc_index();
    virtual char* get_alsa_name();
    virtual void set_alsa_control_name(const char*);

private:
    char *m_alsa_name;
    char m_osc_path[64];
    int m_osc_index;    
};

#endif /* OOSCCONTROL_H */

