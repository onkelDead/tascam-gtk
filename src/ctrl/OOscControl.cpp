/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OOscControl.cpp
 * Author: onkel
 * 
 * Created on 28. Dezember 2024, 09:12
 */

#include "stdio.h"
#include "OOscControl.h"

OOscControl::OOscControl() :
m_alsa_name(0)
{}

OOscControl::~OOscControl() {
    if (m_alsa_name)
        free(m_alsa_name);
}

void OOscControl::osc_init(const char* path) {
    sprintf(m_osc_path, "%s", path);
    m_osc_index = -1;
}

void OOscControl::osc_init(const char* path, int index) {
    sprintf(m_osc_path, "%s/%d", path, index);
    m_osc_index = index;
}

char* OOscControl::get_osc_path() {
    return m_osc_path;
}

int OOscControl::get_osc_index() {
    return m_osc_index;
}

char* OOscControl::get_alsa_name() {
    return m_alsa_name;
}

void OOscControl::set_alsa_control_name(const char* alsa_name) {
    if (m_alsa_name)
        free(m_alsa_name);
    m_alsa_name = strdup(alsa_name);
}
