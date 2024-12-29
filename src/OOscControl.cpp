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

void OOscControl::osc_init(const char* path) {
    sprintf(m_path, "%s", path);
    m_index = -1;
}

void OOscControl::osc_init(const char* path, int index) {
    sprintf(m_path, "%s/%d", path, index);
    m_index = index;
}

char* OOscControl::get_osc_path() {
    return m_path;
}

int OOscControl::get_osc_index() {
    return m_index;
}
