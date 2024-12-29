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
    
    
private:

};

#endif /* OFADER_H */

