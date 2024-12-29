/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ORoute.cpp
 * Author: onkel
 * 
 * Created on 28. Dezember 2024, 18:58
 */

#include <gtkmm-3.0/gtkmm/combobox.h>

#include "ORoute.h"

ORoute::ORoute() : Gtk::ComboBoxText() {
}

ORoute::~ORoute() {
}

void ORoute::osc_init(const char* path) {
    sprintf(m_osc_path, "%s", path);
    m_osc_index = -1;
}

void ORoute::osc_init(const char* path, int index) {
    sprintf(m_osc_path, "%s/%d", path, index);
    m_osc_index = index;
}

char* ORoute::get_osc_path() {
    return m_osc_path;
}

int ORoute::get_osc_index() {
    return m_osc_index;
}

