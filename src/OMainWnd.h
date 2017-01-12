/*
  Copyright 2017 Detlef Urban <onkel@paraair.de>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#ifndef OMAINWND_H
#define OMAINWND_H

#include <queue>
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

    GAsyncQueue *gqueue;
    
    Glib::Mutex oscMutex; 
    std::queue<char*> m_osc_queue;
    void notify_osc();
    void on_notification_from_osc_thread();

    OAlsa *alsa;


    void on_menu_file_load();
    void on_menu_file_save();
    void on_menu_file_reset();
    void on_menu_file_exit();

    void on_menu_popup_load(int i);
    void on_menu_popup_save(int i);
    void on_menu_popup_reset(int i);
    virtual bool on_title_context(GdkEventButton* event, int channel_index);

    void on_ch_fader_changed (int n, const char* control_name, Gtk::VScale* control, Gtk::Label* label);
    void on_ch_dial_changed (int n, const char* control_name, ODial* control);
    void on_ch_tb_changed (int n, const char* control_name, Gtk::ToggleButton* control);
    
    
protected:
//    void on_parsing_error(const Glib::RefPtr<const Gtk::CssSection>& section, const Glib::Error& error);
    Glib::RefPtr<Gtk::CssProvider> m_refCssProvider;


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
    Glib::Dispatcher m_Dispatcher_osc;
    OMeterWorker m_Worker;
    std::thread* m_WorkerThread;

    void on_osc_message(int client_index, const char* path, lo_message msg);
    
};

#endif /* OMAINWND_H */

