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

    
    void on_menu_file_load();
    void on_menu_file_save();
    void on_menu_file_reset();
    void on_menu_file_exit();

    void on_menu_popup_load(int i);
    void on_menu_popup_save(int i);
    void on_menu_popup_reset(int i);
    virtual bool on_title_context(GdkEventButton* event, int channel_index);
    
private:

    
    OMaster m_master;

    Gtk::VBox m_vbox;
    Gtk::VBox m_menubox;
    Gtk::HBox m_hbox;
    Gtk::VSeparator m_sep;

    Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
    Glib::RefPtr<Gtk::UIManager> m_refUIManager;
    
    Gtk::Menu* appMenu;
//    Gtk::MenuItem menuitem_file;
//    Gtk::Menu menu_file;
//    Gtk::ImageMenuItem menuitem_file_load;
//    Gtk::ImageMenuItem menuitem_file_save;
//    Gtk::ImageMenuItem menuitem_file_reset;
//    Gtk::SeparatorMenuItem m_menu_sep;
//    Gtk::ImageMenuItem menuitem_file_exit;

    Gtk::Menu menu_popup;
    Gtk::MenuItem menu_popup_load;
    sigc::connection m_popup_load_connection;
    Gtk::MenuItem menu_popup_save;
    sigc::connection m_popup_save_connection;
    Gtk::MenuItem menu_popup_reset;
    sigc::connection m_popup_reset_connection;

    
    gint open_channels;
    gint last_channel;
    gint cardnum;
    snd_hctl_t *hctl;

    void save_values(Glib::ustring);
    void load_values(Glib::ustring);

    void save_channel_values(Glib::ustring filename, int channel_index);
    void load_channel_values(Glib::ustring, int channel_index);
    
    Glib::Dispatcher m_Dispatcher;
    OMeterWorker m_Worker;
    std::thread* m_WorkerThread;

};

#endif /* OMAINWND_H */

