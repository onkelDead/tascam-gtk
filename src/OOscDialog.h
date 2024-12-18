/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OOscDialog.h
 * Author: onkel
 *
 * Created on 17. November 2024, 13:48
 */


#ifndef OOSCDIALOG_H
#define OOSCDIALOG_H

#include <gtkmm.h>

#include "OConfig.h"

class OOscDialog : public Gtk::Dialog {
public:
    OOscDialog();
    virtual ~OOscDialog();

    void on_btn_cancel_clicked();
    void on_btn_ok_clicked();

    void SetData(OConfig* config);
    void GetData(OConfig* config);
    
    bool GetResult();
    
protected:
    Glib::RefPtr<Gtk::CssProvider> m_refCssProvider;
            
private:

    bool m_result;

    Gtk::Grid m_grid;
    Gtk::Label m_lbl_port;
    Gtk::Entry m_osc_port;
    Gtk::Label m_lbl_no_meters;
    Gtk::CheckButton m_chk_no_meters;
    Gtk::Label m_lbl_full_update;
    Gtk::CheckButton m_chk_full_update;    
};

#endif /* OOSCDIALOG_H */

