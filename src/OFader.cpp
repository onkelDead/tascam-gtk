/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OFader.cpp
 * Author: onkel
 * 
 * Created on 28. Dezember 2024, 15:11
 */

#include <gtkmm-3.0/gtkmm.h>
#include <gtkmm-3.0/gtkmm/widget.h>

#include "OFader.h"

OFader::OFader() : Gtk::VScale() {
}


OFader::~OFader() {
}

void OFader::osc_init(const char* path) {
    sprintf(m_osc_path, "%s", path);
    m_osc_index = -1;
}

void OFader::osc_init(const char* path, int index) {
    sprintf(m_osc_path, "%s/%d", path, index);
    m_osc_index = index;
}

char* OFader::get_osc_path() {
    return m_osc_path;
}

int OFader::get_osc_index() {
    return m_osc_index;
}

void OFader::set_alsa_control_name(const char* alsa_name) {
    sprintf(m_alsa_name, "%s", alsa_name);
}
