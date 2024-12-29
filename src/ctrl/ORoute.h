/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ORoute.h
 * Author: onkel
 *
 * Created on 28. Dezember 2024, 18:58
 */

#ifndef OROUTE_H
#define OROUTE_H

#include <gtkmm/widget.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/styleproperty.h>

#include "OOscControl.h"

#include "OTypes.h"

class ORoute : public Gtk::ComboBoxText , public OOscControl {
public:
    ORoute();
    ORoute(const ORoute& orig);
    virtual ~ORoute();
    
    gint get_value() { return Gtk::ComboBoxText::get_active_row_number(); }
    void set_value(gint val) { Gtk::ComboBoxText::set_active(val); }

private:
};

#endif /* OROUTE_H */

