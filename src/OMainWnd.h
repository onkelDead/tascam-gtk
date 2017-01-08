/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OMainWnd.h
 * Author: onkel
 *
 * Created on January 1, 2017, 2:37 PM
 */

#ifndef OMAINWND_H
#define OMAINWND_H

#include "OStripLayout.h"
#include "OMaster.h"
#include "OAlsa.h"
#include "OMeterWorker.h"


class OMainWnd : public Gtk::Window {
public:
    OMainWnd();
    virtual ~OMainWnd();

    OAlsa* get_alsa() {
        return alsa;
    }

    OStripLayout m_stripLayouts[16];

    void notify();
    void on_notification_from_worker_thread();
    
    OAlsa *alsa;
    
    void on_menu_file_reset(int i);
    void on_menu_file_exit(int i);
    
private:

    
    OMaster m_master;

    Gtk::VBox m_vbox;
    Gtk::HBox m_menubox;
    Gtk::HBox m_hbox;
    Gtk::VSeparator m_sep;

    Gtk::MenuItem menuitem_file;
    Gtk::Menu menu_file;
    Gtk::MenuItem menuitem_file_reset;
    Gtk::MenuItem menuitem_file_exit;
    
    gint open_channels;
    gint last_channel;
    gint cardnum;
    snd_hctl_t *hctl;


  Glib::Dispatcher m_Dispatcher;
  OMeterWorker m_Worker;
  std::thread* m_WorkerThread;
  
};

#endif /* OMAINWND_H */

