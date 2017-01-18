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

#include "OTypes.h"
#include "OStripLayout.h"
#include "ODspLayout.h"
#include "OMaster.h"
#include "ORoute.h"
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
    ODspLayout m_dsp_layout;

    void notify();
    void on_notification_from_worker_thread();

    GAsyncQueue *gqueue;

    Glib::Mutex oscMutex;
    std::queue<char*> m_osc_queue;
    void notify_osc();
    void on_notification_from_osc_thread();

    OAlsa *alsa;

    Gtk::ToggleButton m_comp_enable[NUM_CHANNELS+1];
    ODial m_threshold[NUM_CHANNELS+1];
    ODial m_gain[NUM_CHANNELS+1];
    ODial m_attack[NUM_CHANNELS+1];
    ODial m_release[NUM_CHANNELS+1];
    ODial m_ratio[NUM_CHANNELS+1];
    OMeter m_reduction[NUM_CHANNELS+1];

    Gtk::ToggleButton m_eq_enable[NUM_CHANNELS+1];
    ODial m_high_freq_gain[NUM_CHANNELS+1];
    ODial m_high_freq_band[NUM_CHANNELS+1];
    ODial m_mid_high_freq_gain[NUM_CHANNELS+1];
    ODial m_mid_high_freq_band[NUM_CHANNELS+1];
    ODial m_mid_high_freq_width[NUM_CHANNELS+1];
    ODial m_mid_low_freq_gain[NUM_CHANNELS+1];
    ODial m_mid_low_freq_band[NUM_CHANNELS+1];
    ODial m_mid_low_freq_width[NUM_CHANNELS+1];
    ODial m_low_freq_gain[NUM_CHANNELS+1];
    ODial m_low_freq_band[NUM_CHANNELS+1];

    ODial m_Pan[NUM_CHANNELS+1];
    Gtk::ToggleButton m_PhaseEnable[NUM_CHANNELS+1];
    Gtk::ToggleButton m_MuteEnable[NUM_CHANNELS+1];
    Gtk::ToggleButton m_SoloEnable[NUM_CHANNELS+1];
    Gtk::VScale m_fader[NUM_CHANNELS+1];

    ORoute m_route;
    
    void on_menu_file_load();
    void on_menu_file_save();
    void on_menu_file_reset();
    void on_menu_file_exit();

    void on_menu_view_compact();
    void on_menu_view_normal();

    void on_menu_popup_load(int i);
    void on_menu_popup_save(int i);
    void on_menu_popup_reset(int i);
    virtual bool on_title_context(GdkEventButton* event, int channel_index);

    void on_ch_fader_changed(int n, const char* control_name, Gtk::VScale* control, Gtk::Label* label);
    void on_ch_dial_changed(int n, const char* control_name);
    void on_ch_tb_changed(int n, const char* control_name);
    void set_dsp_channel(int n, bool enable);

    void on_ch_lb_changed(int n);

protected:
    //    void on_parsing_error(const Glib::RefPtr<const Gtk::CssSection>& section, const Glib::Error& error);
    Glib::RefPtr<Gtk::CssProvider> m_refCssProvider;


private:

    bool m_block_ui;
    int m_last_dsp_active;
    
    VIEW_TYPE m_view;

    Gtk::Grid m_grid;

    OMaster m_master;

    Gtk::VBox m_menubox;

    Gtk::ToggleButton m_link[8];

    Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
    Glib::RefPtr<Gtk::UIManager> m_refUIManager;

    Gtk::Menu* appMenu;

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

    void create_menu();
};

#endif /* OMAINWND_H */

