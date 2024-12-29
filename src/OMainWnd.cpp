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

#include <gtkmm.h>
#include <giomm/simpleactiongroup.h>
#include "OMainWnd.h"
#include "config.h"
#include <iostream>
#include <gtkmm-3.0/gtkmm/widget.h>

#ifdef HAVE_OSC
#define OSC_STRIP_INDEX (argv[0]->i - 1)
#define OSC_STRIP_I0 (argv[0]->i32)
#define OSC_STRIP_I1 (argv[1]->i32)
#define OSC_STRIP_B0 (argv[0]->i32 != 0)
#define OSC_STRIP_B1 (argv[1]->i32 != 0)

#define OSC_STRIP_INDEX1(n) (atoi(path+n)-1)

#define STEREO_LEFT_MASK 0x0e
#define SM_INDEX (m_stripLayouts[OSC_STRIP_INDEX & STEREO_LEFT_MASK].get_channel_type() == STEREO ? OSC_STRIP_INDEX & 0x0e : OSC_STRIP_INDEX)
#define SMP_INDEX(n) (m_stripLayouts[OSC_STRIP_INDEX1(n) & STEREO_LEFT_MASK].get_channel_type() == STEREO ? OSC_STRIP_INDEX1(n) & 0x0e : OSC_STRIP_INDEX1(n))

#define OSC_MASTER_MSG(path, value) \
{   \
    lo_message reply = lo_message_new();  \
    lo_message_add_int32(reply, value);   \
    m_Worker.send_osc_all(path, reply);    \
    lo_message_free(reply); \
}


#define OSC_STRIP_MSG1(path, index, value) \
{ \
    char xpath[32]; \
    sprintf(xpath, "%s/%d", path, index);  \
    lo_message reply = lo_message_new();  \
    lo_message_add_int32(reply, value);   \
    m_Worker.send_osc_all(xpath, reply);    \
    lo_message_free(reply); \
}

#define OSC_STRIP_MSG(path, index, value) \
{ \
    lo_message reply = lo_message_new();  \
    lo_message_add_int32(reply, index);   \
    lo_message_add_int32(reply, value);   \
    m_Worker.send_osc_all(path, reply);    \
    lo_message_free(reply); \
}


#define OSC_STRIP_MSG2(path, value) \
{ \
    lo_message reply = lo_message_new();  \
    lo_message_add_int32(reply, value);   \
    m_Worker.send_osc_all(path, reply);    \
    lo_message_free(reply); \
}

#else
#define OSC_STRIP_INDEX 
#define OSC_STRIP_I0 
#define OSC_STRIP_I1
#define OSC_STRIP_B0
#define OSC_STRIP_B1
#define OSC_MASTER_MSG(path, value)
#define OSC_STRIP_MSG(path, index, value)
#endif


/// constructor implementation
OMainWnd::OMainWnd() :
Gtk::Window(),
settings(0),
alsa(0),
m_view(VIEW_TYPE::NORMAL),
m_solo_channel(-1),
m_dsp_channel(-1),
m_block_ui(true),
m_menubar(nullptr),
m_WorkerThread(nullptr),
block_events(0),
m_WorkerAlsaThread(nullptr) {

    bool compact;

    set_name("OMainWnd");
    set_title("Tascam US-16x08 DSP Mixer");

    if (Glib::file_test(PKGDATADIR "/icon.png", Glib::FILE_TEST_EXISTS))
        this->set_icon_from_file(PKGDATADIR "/icon.png");
    else
        this->set_icon_from_file("data/icon.png");

    create_menu();

    this->signal_delete_event().connect(sigc::mem_fun(this, &OMainWnd::on_delete));

    // load style sheet 
    apply_gdk_style();

    // create about dialog
    create_aboutdlg();

    alsa = new OAlsa(this);
    if (alsa->open_device()) {
        auto dialog = new Gtk::MessageDialog(*this, "Unable to access sound card.\nPlease check terminal shell output for more details.", false, Gtk::MessageType::MESSAGE_ERROR);
        dialog->run();
        delete dialog;
        exit(1);
    }

    create_controls();

    show_all_children(true);
    
    l_log_osc = m_config.get_boolean(SETTINGS_OSC_LOG_ALL);
    
    create_worker_threads(); 
    
    if (m_config.get_boolean(SETTINGS_WINDOW_COMPACT))
        on_menu_view_compact();
    else
        on_menu_view_normal();
    
    m_block_ui = false;
}

bool OMainWnd::on_delete(GdkEventAny* event) {
    if (settings)
        settings->set_boolean("view-compact", m_view == COMPACT);
    usleep(1000);
    return false;
}

OMainWnd::~OMainWnd() {
    m_Worker.stop_work();
    while (!m_Worker.has_stopped())
        sleep(1);
    if (m_WorkerThread->joinable())
        m_WorkerThread->join();
    delete m_WorkerThread;
    m_WorkerThread = nullptr;


    if (alsa) {
        alsa->close_device();
        alsa->stop_work();
        //        while (!alsa->has_stopped()) {
        //            sleep(1);
        //        }
        delete alsa;
    }
    if (m_menubar)
        delete m_menubar;
#ifdef HAVE_OSC 
    g_async_queue_unref(m_osc_queue);
#endif
}

void OMainWnd::create_menu() {
    m_menubar = Gtk::manage(new Gtk::MenuBar);

    m_refActionGroup = Gtk::ActionGroup::create();
    m_refActionGroup->add(Gtk::Action::create("File", "_File"));
    m_refActionGroup->add(Gtk::Action::create("load", Gtk::Stock::OPEN, "_Load values..."),
            sigc::mem_fun(this, &OMainWnd::on_menu_file_load));
    m_refActionGroup->add(Gtk::Action::create("save", Gtk::Stock::SAVE, "_Save values..."),
            sigc::mem_fun(this, &OMainWnd::on_menu_file_save));
    m_refActionGroup->add(Gtk::Action::create("reset", Gtk::Stock::REVERT_TO_SAVED, "_Reset all"),
            sigc::mem_fun(this, &OMainWnd::on_menu_file_reset));
    m_refActionGroup->add(Gtk::Action::create("exit", Gtk::Stock::QUIT),
            sigc::mem_fun(this, &OMainWnd::on_menu_file_exit));
    m_refActionGroup->add(Gtk::Action::create("about", Gtk::Stock::ABOUT),
            sigc::mem_fun(this, &OMainWnd::on_menu_file_about));
#ifdef HAVE_OSC
    m_refActionGroup->add(Gtk::Action::create("osc", Gtk::Stock::PREFERENCES, "_OSC settings"),
            sigc::mem_fun(this, &OMainWnd::on_menu_file_osc));
#endif    
    m_refActionGroup->add(Gtk::Action::create("View", "_View"));
    m_refActionGroup->add(Gtk::Action::create("compact", Gtk::Stock::ZOOM_IN, "_Compact"),
            sigc::mem_fun(this, &OMainWnd::on_menu_view_compact));
    m_refActionGroup->add(Gtk::Action::create("normal", Gtk::Stock::ZOOM_OUT, "_Normal"),
            sigc::mem_fun(this, &OMainWnd::on_menu_view_normal));

    m_refUIManager = Gtk::UIManager::create();
    m_refUIManager->insert_action_group(m_refActionGroup);

    //Layout the actions in a menubar and toolbar:
#ifdef HAVE_OSC    
    Glib::ustring ui_info =
            "<ui>"
            "  <menubar name='MenuBar'>"
            "    <menu action='File'>"
            "        <menuitem action='load'/>"
            "        <menuitem action='save'/>"
            "        <menuitem action='reset'/>"
            "        <separator />"
            "        <menuitem action='osc'/>"
            "        <separator />"
            "        <menuitem action='about'/>"
            "        <separator />"
            "        <menuitem action='exit' />"
            "    </menu>"
            "    <menu action='View'>"
            "        <menuitem action='compact'/>"
            "        <menuitem action='normal'/>"
            "    </menu>"
            "  </menubar>"
            "</ui>";
#else
    Glib::ustring ui_info =
            "<ui>"
            "  <menubar name='MenuBar'>"
            "    <menu action='File'>"
            "        <menuitem action='load'/>"
            "        <menuitem action='save'/>"
            "        <menuitem action='reset'/>"
            "        <separator />"
            "        <menuitem action='about'/>"
            "        <separator />"
            "        <menuitem action='exit' />"
            "    </menu>"
            "    <menu action='View'>"
            "        <menuitem action='compact'/>"
            "        <menuitem action='normal'/>"
            "    </menu>"
            "  </menubar>"
            "</ui>";
#endif    

    try {
        m_refUIManager->add_ui_from_string(ui_info);
    } catch (const Glib::Error& ex) {
        std::cerr << "building menus failed: " << ex.what();
    }

    Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");
    if (!(pMenubar)) {
        g_warning("GMenu or AppMenu not found");
    } else {
        m_menubox.pack_start(*pMenubar, false, false);
    }

    menu_popup_load.set_label("Load channel values");
    menu_popup.append(menu_popup_load);
    menu_popup_save.set_label("Save channel values");
    menu_popup.append(menu_popup_save);
    menu_popup_reset.set_label("Reset channel values");
    menu_popup.append(menu_popup_reset);

    m_menubox.set_name("menu");
    m_grid.attach(m_menubox, 0, 0, 17, 1);

}

void OMainWnd::create_controls() {
    for (int i = 0; i < NUM_CHANNELS + 1; i++) {
        // compressor controls
        {
            m_comp_enable[i].set_label("Comp");
            m_comp_enable[i].set_name("comp-button");
            m_comp_enable[i].set_ledcolor(1, .6, .6, 1.);
            m_comp_enable[i].set_halign(Gtk::ALIGN_FILL);
            m_comp_enable[i].set_valign(Gtk::ALIGN_FILL);
            m_comp_enable[i].set_fontsize(7);
            m_comp_enable[i].set_ledsize(6);
            m_comp_enable[i].osc_init("/ch/comp/sw", i + 1);
            add_osc_control(&m_comp_enable[i]);
            
            m_threshold[i].set_params(0, 32, 32, 1);
            m_threshold[i].set_label("Thresh");
            m_threshold[i].set_value_callback(cp_threshold_text);
            m_threshold[i].set_knob_background_color(CREAD_NORMAL);
            m_threshold[i].osc_init("/ch/comp/threshold", i + 1);
            add_osc_control(&m_threshold[i]);

            m_gain[i].set_params(0, 20, 0, 1);
            m_gain[i].set_label("Gain");
            m_gain[i].set_value_callback(cp_gain_text);
            m_gain[i].set_knob_background_color(CREAD_NORMAL);
            m_gain[i].osc_init("/ch/comp/gain", i + 1);
            add_osc_control(&m_gain[i]);

            m_attack[i].set_params(0, 198, 0, 5);
            m_attack[i].set_label("Attack");
            m_attack[i].set_value_callback(cp_attack_text);
            m_attack[i].set_knob_background_color(CREAD_LIGHT);
            m_attack[i].osc_init("/ch/comp/attack", i + 1);
            add_osc_control(&m_attack[i]);

            m_release[i].set_params(0, 99, 0, 1);
            m_release[i].set_label("Release");
            m_release[i].set_value_callback(cp_release_text);
            m_release[i].set_knob_background_color(CREAD_LIGHT);
            m_release[i].osc_init("/ch/comp/release", i + 1);
            add_osc_control(&m_release[i]);

            m_ratio[i].set_params(0, 14, 0, 1);
            m_ratio[i].set_label("Ratio");
            m_ratio[i].set_map(cp_ration_map);
            m_ratio[i].set_knob_background_color(CREAD_NORMAL);
            m_ratio[i].osc_init("/ch/comp/ratio", i + 1);
            add_osc_control(&m_ratio[i]);

            m_reduction[i].setLevel(32768);
            m_reduction[i].set_size_request(10, -1);
            m_reduction[i].set_level_direction(1);
            m_reduction[i].set_level_color(1, .6, .6, 1);
        }

        // equalizer controls
        {
            m_eq_enable[i].set_label("EQ");
            m_eq_enable[i].set_name("eq-switch");
            m_eq_enable[i].set_vexpand(false);
            m_eq_enable[i].set_valign(Gtk::ALIGN_CENTER);
            m_eq_enable[i].set_ledcolor(.6, .6, 1., 1.);
            m_eq_enable[i].set_halign(Gtk::ALIGN_FILL);
            m_eq_enable[i].set_valign(Gtk::ALIGN_FILL);
            m_eq_enable[i].set_fontsize(7);
            m_eq_enable[i].set_ledsize(6);
            m_eq_enable[i].osc_init("/ch/eq/sw", i+1);
            add_osc_control(&m_eq_enable[i]);

            m_lcf_enable[i].set_label("LCF");
            m_lcf_enable[i].set_name("lcf-button");
            m_lcf_enable[i].set_vexpand(false);
            m_lcf_enable[i].set_valign(Gtk::ALIGN_CENTER);
            m_lcf_enable[i].set_ledcolor(.6, .6, 1., 1.);
            m_lcf_enable[i].set_halign(Gtk::ALIGN_FILL);
            m_lcf_enable[i].set_valign(Gtk::ALIGN_FILL);
            m_lcf_enable[i].set_fontsize(7);
            m_lcf_enable[i].set_ledsize(6);
            m_lcf_enable[i].osc_init("/ch/eq/lcf", i+1);
            add_osc_control(&m_lcf_enable[i]);
            
            m_high_freq_gain[i].set_label("High");
            m_high_freq_gain[i].set_value_callback(eq_level_text);
            m_high_freq_gain[i].set_params(0, 24, 12, 1);
            m_high_freq_gain[i].set_name("eq_high_gain");
            m_high_freq_gain[i].set_knob_background_color(EBLUE_NORMAL);
            m_high_freq_gain[i].osc_init("/ch/eq/highgain", i + 1);
            add_osc_control(&m_high_freq_gain[i]);

            m_high_freq_band[i].set_label("Freq");
            m_high_freq_band[i].set_value_callback(eq_high_freq_text);
            m_high_freq_band[i].set_params(0, 31, 15, 1);
            m_high_freq_band[i].set_knob_background_color(EBLUE_LIGHT);
            m_high_freq_band[i].osc_init("/ch/eq/highfreq", i + 1);
            add_osc_control(&m_high_freq_band[i]);

            m_mid_high_freq_gain[i].set_label("Mid H");
            m_mid_high_freq_gain[i].set_params(0, 24, 12, 1);
            m_mid_high_freq_gain[i].set_value_callback(eq_level_text);
            m_mid_high_freq_gain[i].set_knob_background_color(EBLUE_NORMAL);
            m_mid_high_freq_gain[i].osc_init("/ch/eq/midhighgain", i + 1);
            add_osc_control(&m_mid_high_freq_gain[i]);

            m_mid_high_freq_band[i].set_label("Freq");
            m_mid_high_freq_band[i].set_params(0, 63, 27, 1);
            m_mid_high_freq_band[i].set_value_callback(eq_lowhigh_freq_text);
            m_mid_high_freq_band[i].set_knob_background_color(EBLUE_LIGHT);
            m_mid_high_freq_band[i].osc_init("/ch/eq/midhighfreq", i + 1);
            add_osc_control(&m_mid_high_freq_band[i]);

            m_mid_high_freq_width[i].set_label("Width");
            m_mid_high_freq_width[i].set_value_callback(eq_width_text);
            m_mid_high_freq_width[i].set_params(0, 6, 2, 1);
            m_mid_high_freq_width[i].set_knob_background_color(EBLUE_LIGHT);
            m_mid_high_freq_width[i].set_hexpand(false);
            m_mid_high_freq_width[i].set_halign(Gtk::ALIGN_CENTER);
            m_mid_high_freq_width[i].osc_init("/ch/eq/midhighwidth", i + 1);
            add_osc_control(&m_mid_high_freq_width[i]);

            m_mid_low_freq_gain[i].set_label("Mid L");
            m_mid_low_freq_gain[i].set_params(0, 24, 12, 1);
            m_mid_low_freq_gain[i].set_value_callback(eq_level_text);
            m_mid_low_freq_gain[i].set_knob_background_color(EBLUE_NORMAL);
            m_mid_low_freq_gain[i].osc_init("/ch/eq/midlowgain", i + 1);
            add_osc_control(&m_mid_low_freq_gain[i]);

            m_mid_low_freq_band[i].set_label("Freq");
            m_mid_low_freq_band[i].set_params(0, 63, 14, 1);
            m_mid_low_freq_band[i].set_value_callback(eq_lowhigh_freq_text);
            m_mid_low_freq_band[i].set_knob_background_color(EBLUE_LIGHT);
            m_mid_low_freq_band[i].osc_init("/ch/eq/midlowfreq", i + 1);
            add_osc_control(&m_mid_low_freq_band[i]);

            m_mid_low_freq_width[i].set_label("Width");
            m_mid_low_freq_width[i].set_value_callback(eq_width_text);
            m_mid_low_freq_width[i].set_params(0, 6, 2, 1);
            m_mid_low_freq_width[i].set_knob_background_color(EBLUE_LIGHT);
            m_mid_low_freq_width[i].osc_init("/ch/eq/midlowwidth", i + 1);
            add_osc_control(&m_mid_low_freq_width[i]);

            m_low_freq_gain[i].set_label("Low");
            m_low_freq_gain[i].set_params(0, 24, 12, 1);
            m_low_freq_gain[i].set_value_callback(eq_level_text);
            m_low_freq_gain[i].set_knob_background_color(EBLUE_NORMAL);
            m_low_freq_gain[i].osc_init("/ch/eq/lowgain", i + 1);
            add_osc_control(&m_low_freq_gain[i]);

            m_low_freq_band[i].set_label("Freq");
            m_low_freq_band[i].set_params(0, 31, 5, 1);
            m_low_freq_band[i].set_value_callback(eq_low_freq_text);
            m_low_freq_band[i].set_knob_background_color(EBLUE_LIGHT);
            m_low_freq_band[i].osc_init("/ch/eq/lowfreq", i + 1);
            add_osc_control(&m_low_freq_band[i]);
        }

        if (i < NUM_CHANNELS) {
            m_Pan[i].set_params(0, 254, 127, 5);
            m_Pan[i].set_label("L Pan R");
            m_Pan[i].set_knob_background_color(1., .8, .3, 1.);
            m_Pan[i].osc_init("/ch/pan", i + 1);
            add_osc_control(&m_Pan[i]);

            m_MuteEnable[i].set_label("Mute");
            m_MuteEnable[i].set_name("mute-button");
            m_MuteEnable[i].set_ledcolor(1., 0., 0., 1.);
            m_MuteEnable[i].set_halign(Gtk::ALIGN_FILL);
            m_MuteEnable[i].set_valign(Gtk::ALIGN_FILL);
            m_MuteEnable[i].set_fontsize(7);
            m_MuteEnable[i].set_ledsize(6);
            m_MuteEnable[i].osc_init("/ch/mute", i + 1);
            add_osc_control(&m_MuteEnable[i]);

            m_SoloEnable[i].set_label("Solo");
            m_SoloEnable[i].set_name("solo-button");
            m_SoloEnable[i].set_ledcolor(1., .5, 0., 1.);
            m_SoloEnable[i].set_halign(Gtk::ALIGN_FILL);
            m_SoloEnable[i].set_valign(Gtk::ALIGN_FILL);
            m_SoloEnable[i].set_fontsize(7);
            m_SoloEnable[i].set_ledsize(6);
            m_SoloEnable[i].osc_init("/ch/solo", i + 1);
            m_SoloEnable[i].signal_switched.connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_toggle_solo), i));
            add_osc_control(&m_SoloEnable[i]);

            m_PhaseEnable[i].set_label("Phase");
            m_PhaseEnable[i].set_name("phase-button");
            m_PhaseEnable[i].set_ledcolor(0., 1., 1., 1.);
            m_PhaseEnable[i].set_halign(Gtk::ALIGN_FILL);
            m_PhaseEnable[i].set_valign(Gtk::ALIGN_FILL);
            m_PhaseEnable[i].set_fontsize(7);
            m_PhaseEnable[i].set_ledsize(6);
            m_PhaseEnable[i].osc_init("/ch/phase", i + 1);
            add_osc_control(&m_PhaseEnable[i]);

            m_fader[i].set_range(0, 133);
            m_fader[i].set_name("fader");
            m_fader[i].set_inverted(true);
            m_fader[i].set_size_request(-1, 160);
            m_fader[i].set_draw_value(false);
            m_fader[i].set_increments(1, 5);
//            m_fader[i].add_mark(133, Gtk::PositionType::POS_RIGHT, "+6 dB");
//            m_fader[i].add_mark(123, Gtk::PositionType::POS_RIGHT, "+3 dB");
//            m_fader[i].add_mark(113, Gtk::PositionType::POS_RIGHT, "0 dB");
//            m_fader[i].add_mark(89, Gtk::PositionType::POS_RIGHT, "-10 dB");
//            m_fader[i].add_mark(73, Gtk::PositionType::POS_RIGHT, "-20 dB");
//            m_fader[i].add_mark(50, Gtk::PositionType::POS_RIGHT, "-40 dB");
//            m_fader[i].add_mark(34, Gtk::PositionType::POS_RIGHT, "-60 dB");
//            m_fader[i].add_mark(16, Gtk::PositionType::POS_RIGHT, "-90 dB");
//            m_fader[i].add_mark(0, Gtk::PositionType::POS_RIGHT, "-inf dB");
            m_fader[i].set_tooltip_text("channel fader");
            m_fader[i].set_vexpand(true);

            m_stripLayouts[i].init(i, alsa, this);
            m_stripLayouts[i].m_event_box.signal_button_press_event().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_mouse_event), i));
            m_stripLayouts[i].set_view_type(NORMAL);
            m_grid.attach(m_stripLayouts[i], i, 2, 1, 1);
        }
    }
    for (int i = 0; i < NUM_CHANNELS / 2; i++) {
        m_link[i].set_label("Link");
        m_link[i].set_name("link-button");
        m_link[i].signal_switched.connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_control_changed), &m_link[i]));
        m_link[i].signal_switched.connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_ch_lb_changed), i));
        
        m_link[i].set_ledcolor(1., 1., 0., 1.);
        m_link[i].set_ledsize(6);
        m_link[i].set_fontsize(8);
        m_link[i].set_align(Gtk::Align::ALIGN_CENTER);
        m_link[i].set_halign(Gtk::ALIGN_FILL);
        m_link[i].osc_init("/link", i + 1);
        add_osc_control(&m_link[i]);
        m_grid.attach(m_link[i], i * 2, 3, 2, 1);
    }

    add_osc_control(&m_master.m_comp_to_stereo);
    add_osc_control(&m_master.m_true_bypass);
    add_osc_control(&m_master.m_mute);

    
    // create DSP layout
    {
        m_dsp_layout.init(16, alsa, this);
        m_dsp_layout.set_view_type(PREPARE);
        m_dsp_layout.set_sensitive(false);
        //		m_grid.attach(m_dsp_layout, 0, 1, 16, 1);
    }
    
    m_routing.init(alsa, this);
    m_master.init(alsa, this);
    m_grid.attach(m_master, 16, 1, 1, 3);

    add(m_grid);
}

void OMainWnd::create_aboutdlg() {
    {
        m_Dialog.set_transient_for(*this);

        if (Glib::file_test(PKGDATADIR "/icon.png", Glib::FILE_TEST_EXISTS))
            m_Dialog.set_logo(Gdk::Pixbuf::create_from_file(PKGDATADIR "/icon.png"));
        else
            m_Dialog.set_logo(Gdk::Pixbuf::create_from_file("data/icon.png"));

        m_Dialog.set_program_name(PACKAGE_STRING);
        m_Dialog.set_version(PACKAGE_VERSION);
        m_Dialog.set_copyright("Copyright 2017 Detlef Urban");
        m_Dialog.set_comments("Tascam US-16x08 DSP mixer application");
        m_Dialog.set_license("MIT");

        m_Dialog.set_website("http://www.paraair.de/tascamgtk");
        m_Dialog.set_website_label("http://www.paraair.de");

        std::vector<Glib::ustring> list_authors;
        list_authors.push_back("Detlef Urban (" PACKAGE_BUGREPORT ")");
        m_Dialog.set_authors(list_authors);

        m_Dialog.signal_response().connect(
                sigc::mem_fun(*this, &OMainWnd::on_about_dialog_response));

        show_all_children();
    }
}

void OMainWnd::apply_gdk_style() {
    {
        m_refCssProvider = Gtk::CssProvider::create();
        auto refStyleContext = get_style_context();
        refStyleContext->add_provider(m_refCssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

        try {
            if (Glib::file_test(PKGDATADIR "/tascam-gtk.css", Glib::FILE_TEST_EXISTS))
                m_refCssProvider->load_from_path(PKGDATADIR "/tascam-gtk.css");
            else
                m_refCssProvider->load_from_path("./data/tascam-gtk.css");
        } catch (const Gtk::CssProviderError& ex) {
            std::cerr << "CssProviderError, Gtk::CssProvider::load_from_path() failed: "
                    << ex.what() << std::endl;
        } catch (const Glib::Error& ex) {
            std::cerr << "Error, Gtk::CssProvider::load_from_path() failed: "
                    << ex.what() << std::endl;
        }

        //	auto refStyleContext = get_style_context();
        auto screen = Gdk::Screen::get_default();
        refStyleContext->add_provider_for_screen(Gdk::Screen::get_default(), m_refCssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

void OMainWnd::create_worker_threads(){
    m_Dispatcher.connect(sigc::mem_fun(*this, &OMainWnd::on_notification_from_worker_thread));
    m_Dispatcher_alsa.connect(sigc::mem_fun(*this, &OMainWnd::on_notification_from_alsa_thread));
#ifdef HAVE_OSC
    m_osc_queue = g_async_queue_new();
    m_Dispatcher_osc.connect(sigc::mem_fun(*this, &OMainWnd::on_notification_from_osc_thread));
#endif
    if (m_WorkerThread) {
        std::cout << "Can't start a worker thread while another one is running." << std::endl;
    } else {
        // Start a new worker thread.
        m_WorkerThread = new std::thread([this] {
            m_Worker.do_work(this);
        });
    }
    
    if (m_WorkerAlsaThread) {
        std::cout << "Can't start a worker thread while another one is running." << std::endl;
    } else {
        m_WorkerAlsaThread = new std::thread([this] {
            alsa->do_work(this);
            this->hide();
        });
    }
}

alsa_control* OMainWnd::get_alsa_widget(const char* info_name, int index, snd_ctl_elem_type_t t) {
    alsa_control* ac = 0;
    if (strcmp(info_name, "Master Mute Switch") == 0) {
        ac = new alsa_control;
        ac->type = Switch;
        ac->oswitch = &m_master.m_mute;
    }
    else if (strcmp(info_name, "Master Volume") == 0) {
        ac = new alsa_control;
        ac->type = Fader;
        ac->faderwidget = &m_master.m_fader;        
    }
    else if (strcmp(info_name, "DSP Bypass Switch") == 0) {
        ac = new alsa_control;
        ac->type = Switch;
        ac->oswitch = &m_master.m_true_bypass;
    }
    else if (strcmp(info_name, "Buss Out Switch") == 0) {
        ac = new alsa_control;
        ac->type = Switch;
        ac->oswitch = &m_master.m_comp_to_stereo;
    }
    else if (strcmp(info_name, "Line Out Route") == 0) {
        ac = new alsa_control;
        ac->type = ComboBox;
        ac->combo = &m_routing.m_route[index];        
    }   
    else if (strcmp(info_name, "Line Volume") == 0) {
        ac = new alsa_control;
        ac->type = Fader;
        ac->faderwidget = &m_fader[index];
    }   
    else if (strcmp(info_name, "Compressor Attack") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_attack[index];
    }  
    else if (strcmp(info_name, "Compressor Ratio") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_ratio[index];
    }
    else if (strcmp(info_name, "Compressor Release") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_release[index];
    }
    else if (strcmp(info_name, "Compressor Volume") == 0) {
        if (t == SND_CTL_ELEM_TYPE_INTEGER) {
            ac = new alsa_control;
            ac->type = Dial;
            ac->dial = &m_gain[index];
        }
    }
    else if (strcmp(info_name, "Compressor Switch") == 0) {
        ac = new alsa_control;
        ac->type = Switch;
        ac->oswitch = &m_comp_enable[index];
    }
    else if (strcmp(info_name, "Compressor Threshold Volume") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_threshold[index];
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
    else if (strcmp(info_name, "EQ High Frequency") == 0) {
#else
    else if (strcmp(info_name, "EQ High Frequence") == 0) {
#endif
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_high_freq_band[index];
    }
    else if (strcmp(info_name, "EQ High Volume") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_high_freq_gain[index];
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
    else if (strcmp(info_name, "EQ Low Frequency") == 0) {
#else
    else if (strcmp(info_name, "EQ Low Frequence") == 0) {
#endif
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_low_freq_band[index];
    }
    else if (strcmp(info_name, "EQ Low Volume") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_low_freq_gain[index];
    }    
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
    else if (strcmp(info_name, "EQ MidHigh Frequency") == 0) {
#else
    else if (strcmp(info_name, "EQ MidHigh Frequence") == 0) {
#endif
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_mid_high_freq_band[index];
    }
    else if (strcmp(info_name, "EQ MidHigh Volume") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_mid_high_freq_gain[index];
    }
    else if (strcmp(info_name, "EQ MidHigh Q") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_mid_high_freq_width[index];
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
    else if (strcmp(info_name, "EQ MidLow Frequency") == 0) {
#else
    else if (strcmp(info_name, "EQ MidLow Frequence") == 0) {
#endif
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_mid_low_freq_band[index];
    }
    else if (strcmp(info_name, "EQ MidLow Volume") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_mid_low_freq_gain[index];
    }
    else if (strcmp(info_name, "EQ MidLow Q") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_mid_low_freq_width[index];
    }
    else if (strcmp(info_name, "EQ Switch") == 0) {
        ac = new alsa_control;
        ac->type = Switch;
        ac->oswitch = &m_eq_enable[index];
    }
    else if (strcmp(info_name, "LCF Switch") == 0) {
        ac = new alsa_control;
        ac->type = Switch;
        ac->oswitch = &m_lcf_enable[index];
    }    
    else if (strcmp(info_name, "Mute Switch") == 0) {
        ac = new alsa_control;
        ac->type = Switch;
        ac->oswitch = &m_MuteEnable[index];
    }
    else if (strcmp(info_name, "Pan Left-Right Volume") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_Pan[index];
    }
    else if (strcmp(info_name, "Phase Switch") == 0) {
        ac = new alsa_control;
        ac->type = Switch;
        ac->oswitch = &m_PhaseEnable[index];
    }
    return ac;
}

void OMainWnd::alsa_update_control(snd_hctl_elem_t *helem, int val, unsigned int index) {
    alsa_control* widget = m_mixer_elems[helem];
    if (widget) {

        widget->value = val;
        m_alsa_queue.push(widget);
        m_Dispatcher_alsa.emit();
    }
}

void OMainWnd::alsa_add_control(snd_hctl_elem_t *helem) {
    int err = 0;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_value_t *control;
    const char *info_name;
    int control_index;
    
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_value_alloca(&control);

    if ((err = snd_hctl_elem_info(helem, info)) < 0) {
        fprintf(stderr, "Info %s element read error: %s\n", "hw:0", snd_strerror(err));
        return;
    }
    if ((err = snd_hctl_elem_read(helem, control)) < 0) {
        fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
        return;
    }
    info_name = snd_ctl_elem_info_get_name(info);
    control_index = snd_ctl_elem_value_get_index(control);
    alsa_control* widget = get_alsa_widget(info_name, control_index, snd_ctl_elem_info_get_type(info));
    if (widget) {
        m_mixer_elems[helem] = widget;
    }
}

void OMainWnd::on_notification_from_alsa_thread() {
    alsa_control *cv = m_alsa_queue.front();
    m_alsa_queue.pop();
    
    alsa_control* widget = cv;
    if (widget) {
        block_events = true;
        m_block_ui = true;
        switch(widget->type) {
            case Fader:
                widget->faderwidget->set_value(alsa->dBToSlider(cv->value) + 1);
                break;    
            case ComboBox:
                widget->combo->set_active(cv->value);
                break;
            case Dial:
                widget->dial->set_value(cv->value);
                break;
            case Switch:
                widget->oswitch->set_value(cv->value);
                break;
        }
        block_events = false;
        m_block_ui = false;
    }        
    ;
}

void OMainWnd::notify() {
    m_Dispatcher.emit();
}

bool OMainWnd::on_mouse_event(GdkEventButton* event, int channel_index) {
    if (event->button == 3) {

        if (!m_popup_load_connection.empty())
            m_popup_load_connection.disconnect();
        if (!m_popup_save_connection.empty())
            m_popup_save_connection.disconnect();
        if (!m_popup_reset_connection.empty())
            m_popup_reset_connection.disconnect();

        m_popup_load_connection = menu_popup_load.signal_activate().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_menu_popup_load), channel_index));
        m_popup_save_connection = menu_popup_save.signal_activate().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_menu_popup_save), channel_index));
        m_popup_reset_connection = menu_popup_reset.signal_activate().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_menu_popup_reset), channel_index));


        menu_popup.show_all();
        menu_popup.popup(3, event->time);

        return true;
    }

    return false;
}

void OMainWnd::on_notification_from_worker_thread() {
    if (m_WorkerThread && m_Worker.has_stopped()) {
        if (m_WorkerThread->joinable())
            m_WorkerThread->join();
        delete m_WorkerThread;
        m_WorkerThread = nullptr;
    }
    for (int i = 0; i < NUM_CHANNELS; i++) {
        int ch_meter = alsa->sliderTodB(alsa->meters[i] / 32768. * 133.) / 133. * 32768;
        m_stripLayouts[i].m_fader.m_meter[0].setLevel(ch_meter);

        if (m_comp_enable[i].get_value())
            m_reduction[i].setLevel(alsa->sliderTodB(alsa->meters[i + 18] / 32768. * 133.) / 133. * 32768);
        else {
            m_reduction[i].setLevel(32767);
        }

        if (m_stripLayouts[i].get_channel_type() == STEREO) {
            ch_meter = alsa->sliderTodB(alsa->meters[i + 1] / 32768. * 133.) / 133. * 32768;
            m_stripLayouts[i].m_fader.m_meter[1].setLevel(ch_meter);
        }
        if (!m_config.get_boolean(SETTINGS_OSC_NO_METERS)) {
            OSC_STRIP_MSG("/strip/meter", i + 1, m_stripLayouts[i].m_fader.m_meter[0].get_level());
            if (m_stripLayouts[i].m_comp.m_enable->get_value()) {
                OSC_STRIP_MSG("/strip/comp/red", i + 1, m_stripLayouts[i].m_comp.m_reduction[0]->get_level());
            }
        }
    }

    int left_level = alsa->sliderTodB(alsa->meters[16] / 32768. * 133.) / 133. * 32768;
    int right_level = alsa->sliderTodB(alsa->meters[17] / 32768. * 133.) / 133. * 32768;
    m_master.m_meter_left.setLevel(left_level);
    m_master.m_meter_right.setLevel(right_level);

    int master_leds = MAX(m_master.m_meter_left.get_level(), m_master.m_meter_right.get_level()) * 14 / 32768;
    int led_mask = 1 << master_leds;

    if (!m_config.get_boolean(SETTINGS_OSC_NO_METERS)) {
        OSC_MASTER_MSG("/master/meter/left", m_master.m_meter_left.get_level());
        OSC_MASTER_MSG("/master/meter/right", m_master.m_meter_right.get_level());
    }
}

void OMainWnd::on_menu_file_exit() {
    this->hide();
}

void OMainWnd::on_menu_file_about() {
    m_Dialog.show();
}

void OMainWnd::on_menu_file_osc() {
    m_OscDialog.SetData(&m_config);

    m_OscDialog.run();
    
    if (m_OscDialog.GetResult()) {
        m_OscDialog.GetData(&m_config);
        l_log_osc = m_config.get_boolean(SETTINGS_OSC_LOG_ALL);
    }
}

void OMainWnd::on_menu_file_reset() {
    m_master.reset(alsa);

    m_routing.reset(alsa);

    for (int i = 0; i < NUM_CHANNELS; i++) {
        m_stripLayouts[i].reset(alsa, i);
    }
}

void OMainWnd::on_menu_file_save() {

    Gtk::FileChooserDialog dialog("Please choose a file",
            Gtk::FILE_CHOOSER_ACTION_SAVE);

    dialog.set_current_folder("./");
    dialog.set_transient_for(*this);

    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("Tascam values files");
    filter_text->add_mime_type("text/xml");
    dialog.add_filter(filter_text);

    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Select", Gtk::RESPONSE_OK);

    int result = dialog.run();

    switch (result) {
        case(Gtk::RESPONSE_OK):
            save_values(dialog.get_filename());
            break;
    }

}

void OMainWnd::on_menu_file_load() {

    Gtk::FileChooserDialog dialog("Please choose a file",
            Gtk::FILE_CHOOSER_ACTION_OPEN);

    dialog.set_current_folder("./");
    dialog.set_transient_for(*this);

    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("Tascam values files");
    filter_text->add_mime_type("text/xml");
    dialog.add_filter(filter_text);

    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Select", Gtk::RESPONSE_OK);

    int result = dialog.run();

    dialog.hide();
    switch (result) {
        case(Gtk::RESPONSE_OK):
            load_values(dialog.get_filename());
            break;
    }
}

void OMainWnd::on_menu_view_compact() {
    m_view = VIEW_TYPE::COMPACT;
    for (int i = 0; i < NUM_CHANNELS / 2; i++) {
        on_ch_lb_changed(i);
    }
    m_master.set_view_type(HIDDEN);
    m_master.set_view_type(m_view);
    if (!m_dsp_layout.get_parent())
        m_grid.attach(m_dsp_layout, 0, 1, 16, 1);
    m_dsp_layout.set_view_type(HIDDEN);
    m_dsp_layout.set_view_type(SINGLE_DSP);
    show_all_children(true);
    m_config.set_boolean(SETTINGS_WINDOW_COMPACT, true);
}

void OMainWnd::on_menu_view_normal() {
    if (m_view == COMPACT && m_dsp_channel != -1)
        set_dsp_channel(m_dsp_channel, false);

    m_view = VIEW_TYPE::NORMAL;
    m_dsp_layout.set_view_type(HIDDEN);
    for (int i = 0; i < NUM_CHANNELS / 2; i++) {
        on_ch_lb_changed(i);
    }
    m_master.set_view_type(HIDDEN);
    m_master.set_view_type(m_view);
    if (m_dsp_layout.get_parent())
        m_grid.remove(m_dsp_layout);
    show_all_children(true);
    m_config.set_boolean(SETTINGS_WINDOW_COMPACT, false);
}

void OMainWnd::on_menu_popup_load(int channel_index) {
    char l_title[256];

    snprintf(l_title, sizeof (l_title), "Select Mixer file to load in %s", m_stripLayouts[channel_index].m_title.get_label().c_str());

    Gtk::FileChooserDialog dialog(l_title, Gtk::FILE_CHOOSER_ACTION_OPEN);

    dialog.set_current_folder("./");
    dialog.set_transient_for(*this);

    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("Tascam values files");
    filter_text->add_mime_type("text/xml");
    dialog.add_filter(filter_text);

    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Select", Gtk::RESPONSE_OK);

    int result = dialog.run();

    dialog.hide();
    switch (result) {
        case(Gtk::RESPONSE_OK):
            load_channel_values(dialog.get_filename(), channel_index);
            break;
    }

}

void OMainWnd::on_menu_popup_save(int channel_index) {
    printf("save %d\n", channel_index);

    Gtk::FileChooserDialog dialog("Please choose a file",
            Gtk::FILE_CHOOSER_ACTION_SAVE);

    dialog.set_current_folder("./");
    dialog.set_transient_for(*this);

    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("Tascam values files");
    filter_text->add_mime_type("text/xml");
    dialog.add_filter(filter_text);

    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Select", Gtk::RESPONSE_OK);

    int result = dialog.run();

    switch (result) {
        case(Gtk::RESPONSE_OK):
            save_channel_values(dialog.get_filename(), channel_index);
            break;
    }

}

void OMainWnd::on_menu_popup_reset(int i) {
    m_stripLayouts[i].reset(alsa, i);
    if (m_stripLayouts[i].get_channel_type() == STEREO) {
        m_stripLayouts[i + 1].reset(alsa, i);
    }
}

void OMainWnd::save_channel_values(Glib::ustring filename, int channel_index) {
    if (!strstr(filename.c_str(), ".xml"))
        filename.append(".xml");

    FILE* file = fopen(filename.c_str(), "w");
    if (file) {
        fprintf(file, "<channel>\n");
        m_stripLayouts[channel_index].save_values(file, 1);
        fprintf(file, "</channel>\n");
        fclose(file);
    }
}

void OMainWnd::load_channel_values(Glib::ustring filename, int channel_index) {

    try {
        xmlpp::TextReader reader(filename);

        while (reader.read()) {
            if (!strcmp(reader.get_name().c_str(), "channel") && reader.get_node_type() != XML_ENDELEMENT) {
                m_stripLayouts[channel_index].load_values(reader.read_outer_xml());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return;
    }

}

void OMainWnd::save_values(Glib::ustring filename) {

    if (!strstr(filename.c_str(), ".xml"))
        filename.append(".xml");

    printf("save: %s\n", filename.c_str());

    FILE* file = fopen(filename.c_str(), "w");
    if (file) {
        fprintf(file, "<values>\n");

        for (int i = 0; i < 8; i++) {

            fprintf(file, "\t<link index=\"%d\">", i);
            fprintf(file, "%d", (int) m_link[i].get_value());
            fprintf(file, "</link>\n");

        }

        fprintf(file, "\t<master>");
        fprintf(file, "%d", (int) m_master.m_fader.get_value());
        fprintf(file, "</master>\n");

        fprintf(file, "\t<mute>");
        fprintf(file, "%d", (int) m_master.m_mute.get_value());
        fprintf(file, "</mute>\n");

        fprintf(file, "\t<bypass>");
        fprintf(file, "%d", (int) m_master.m_true_bypass.get_value());
        fprintf(file, "</bypass>\n");

        fprintf(file, "\t<bus_out>");
        fprintf(file, "%d", (int) m_master.m_comp_to_stereo.get_value());
        fprintf(file, "</bus_out>\n");

        for (int i = 0; i < 8; i++) {

            fprintf(file, "\t<route index=\"%d\">", i);
            fprintf(file, "%d", (int) m_routing.m_route[i].get_active_row_number());
            fprintf(file, "</route>\n");

        }

        for (int j = 0; j < NUM_CHANNELS; j++) {
            fprintf(file, "\t<channel index=\"%d\">\n", j);
            m_stripLayouts[j].save_values(file);
            fprintf(file, "\t</channel>\n");
        }


        fprintf(file, "</values>\n");
        fclose(file);
    }

}

void OMainWnd::load_values(Glib::ustring filename) {

    try {
        xmlpp::TextReader reader(filename);

        while (reader.read()) {
            if (!strcmp(reader.get_name().c_str(), "link") && reader.get_node_type() != XML_ENDELEMENT) {
                if (reader.has_attributes()) {
                    reader.move_to_first_attribute();
                    int index = atoi(reader.get_value().c_str());
                    reader.read();
                    m_link[index].set_value(atoi(reader.get_value().c_str()));
                    usleep(RESET_VALUE_DELAY);
                }
            }
            if (!strcmp(reader.get_name().c_str(), "master") && reader.get_node_type() != XML_ENDELEMENT) {
                reader.read();
                m_master.m_fader.set_value(atoi(reader.get_value().c_str()));
                usleep(RESET_VALUE_DELAY);
            }
            if (!strcmp(reader.get_name().c_str(), "mute") && reader.get_node_type() != XML_ENDELEMENT) {
                reader.read();
                m_master.m_mute.set_value(atoi(reader.get_value().c_str()));
                usleep(RESET_VALUE_DELAY);
            }
            if (!strcmp(reader.get_name().c_str(), "bypass") && reader.get_node_type() != XML_ENDELEMENT) {
                reader.read();
                m_master.m_true_bypass.set_value(atoi(reader.get_value().c_str()));
                usleep(RESET_VALUE_DELAY);
            }
            if (!strcmp(reader.get_name().c_str(), "bus_out") && reader.get_node_type() != XML_ENDELEMENT) {
                reader.read();
                m_master.m_comp_to_stereo.set_value(atoi(reader.get_value().c_str()));
                usleep(RESET_VALUE_DELAY);
            }
            if (!strcmp(reader.get_name().c_str(), "route") && reader.get_node_type() != XML_ENDELEMENT) {
                if (reader.has_attributes()) {
                    reader.move_to_first_attribute();
                    int index = atoi(reader.get_value().c_str());
                    reader.read();
                    m_routing.m_route[index].set_active(atoi(reader.get_value().c_str()));
                    usleep(RESET_VALUE_DELAY);
                }
            }
            if (!strcmp(reader.get_name().c_str(), "channel") && reader.get_node_type() != XML_ENDELEMENT) {
                if (reader.has_attributes()) {
                    reader.move_to_first_attribute();
                    int index = atoi(reader.get_value().c_str());
                    m_stripLayouts[index].load_values(reader.read_outer_xml());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return;
    }

}

#ifdef HAVE_OSC

void OMainWnd::notify_osc() {
    m_Dispatcher_osc.emit();
}

void OMainWnd::update_osc_client() {
    OSC_MASTER_MSG("/master/fader", m_master.m_fader.get_value());
    OSC_MASTER_MSG("/master/bypass", m_master.m_true_bypass.get_value() ? 1 : 0);
    OSC_MASTER_MSG("/master/busout", m_master.m_comp_to_stereo.get_value() ? 1 : 0);
    for (int n = 0; n < 8; n++) {
        OSC_STRIP_MSG("/link", n + 1, m_link[n].get_value() ? 1 : 0);  
        OSC_STRIP_MSG("/master/route", n + 1, m_routing.m_route[n].get_active_row_number());
    }
    for (int n = 0; n < 16; n++) {
        OSC_STRIP_MSG("/strip/fader", n + 1, m_stripLayouts[n].m_fader.m_fader->get_value());
        OSC_STRIP_MSG("/strip/mute", n + 1, m_stripLayouts[n].m_fader.m_MuteEnable->get_value() ? 1 : 0);
        OSC_STRIP_MSG("/strip/solo", n + 1, m_stripLayouts[n].m_fader.m_SoloEnable->get_value() ? 1 : 0);
        OSC_STRIP_MSG("/strip/phase", n + 1, m_stripLayouts[n].m_fader.m_PhaseEnable[0]->get_value() ? 1 : 0);
        OSC_STRIP_MSG("/strip/pan", n + 1, m_stripLayouts[n].m_fader.m_Pan[0]->get_value() - 127);
        
        OSC_STRIP_MSG("/strip/eq/active", n + 1, m_stripLayouts[n].m_eq.m_eq_enable->get_value() ? 1 : 0);
        OSC_STRIP_MSG("/strip/eq/highfreq", n + 1, m_stripLayouts[n].m_eq.m_high_freq_band->get_value());
        OSC_STRIP_MSG("/strip/eq/highgain", n + 1, m_stripLayouts[n].m_eq.m_high_freq_gain[0].get_value());
        OSC_STRIP_MSG("/strip/eq/midhighfreq", n + 1, m_stripLayouts[n].m_eq.m_mid_high_freq_band[0].get_value());
        OSC_STRIP_MSG("/strip/eq/midhighgain", n + 1, m_stripLayouts[n].m_eq.m_mid_high_freq_gain[0].get_value());
        OSC_STRIP_MSG("/strip/eq/midhighwidth", n + 1, m_stripLayouts[n].m_eq.m_mid_high_freq_width[0].get_value());
        OSC_STRIP_MSG("/strip/eq/midlowfreq", n + 1, m_stripLayouts[n].m_eq.m_mid_low_freq_band[0].get_value());
        OSC_STRIP_MSG("/strip/eq/midlowgain", n + 1, m_stripLayouts[n].m_eq.m_mid_low_freq_gain[0].get_value());
        OSC_STRIP_MSG("/strip/eq/midlowwidth", n + 1, m_stripLayouts[n].m_eq.m_mid_low_freq_width[0].get_value());
        OSC_STRIP_MSG("/strip/eq/lowfreq", n + 1, m_stripLayouts[n].m_eq.m_low_freq_band[0].get_value());
        OSC_STRIP_MSG("/strip/eq/lowgain", n + 1, m_stripLayouts[n].m_eq.m_low_freq_gain[0].get_value());
        OSC_STRIP_MSG("/strip/eq/lcf", n + 1, m_stripLayouts[n].m_eq.m_lcf_enable->get_value() ? 1 : 0);
        
        OSC_STRIP_MSG("/strip/comp/active", n + 1, m_stripLayouts[n].m_comp.m_enable->get_value() ? 1 : 0);
        OSC_STRIP_MSG("/strip/comp/threshold", n + 1, m_stripLayouts[n].m_comp.m_threshold[0].get_value());
        OSC_STRIP_MSG("/strip/comp/gain", n + 1, m_stripLayouts[n].m_comp.m_gain[0].get_value());
        OSC_STRIP_MSG("/strip/comp/attack", n + 1, m_stripLayouts[n].m_comp.m_attack[0].get_value());
        OSC_STRIP_MSG("/strip/comp/release", n + 1, m_stripLayouts[n].m_comp.m_release[0].get_value());
        OSC_STRIP_MSG("/strip/comp/ratio", n + 1, m_stripLayouts[n].m_comp.m_ratio[0].get_value());
        
    }    
}

void OMainWnd::on_notification_from_osc_thread() {

    oscMutex.lock();

    osc_message* data = (osc_message*) g_async_queue_pop(m_osc_queue);

    oscMutex.unlock();
    if (data->path) {
        on_osc_message(data->client_index, data->path, data->data);
        free(data->path);
    }
    lo_message_free(data->data);
    delete data;
}

void OMainWnd::on_osc_message(int client_index, const char* path, lo_message msg) {
    lo_arg** argv = lo_message_get_argv(msg);
    lo_message reply;

#ifdef OSC_LOG_MSG
    printf("rec:%s", path);
    lo_message_pp(msg);
#endif
    
    OOscControl* osc_sw = m_osc_control_map[path];
    if (osc_sw) {
        osc_sw->set_value(OSC_STRIP_I0);
        return;
    }
    
    
// Master 
    if (!strcmp(path, "/reset"))                on_menu_file_reset();
    if (!strcmp(path, "/master/fader"))         m_master.m_fader.set_value(OSC_STRIP_I0);
//    if (!strcmp(path, "/master/mute"))          m_master.m_mute.set_active(OSC_STRIP_B0);
//    if (!strcmp(path, "/master/bypass"))        m_master.m_true_bypass.set_active(OSC_STRIP_B0);
//    if (!strcmp(path, "/master/busout"))        m_master.m_comp_to_stereo.set_active(OSC_STRIP_B0);

// Routing
    if (!strcmp(path, "/master/route")) {
        int route_index = argv[0]->i - 1;
        int val = argv[1]->i;

        m_routing.m_route[route_index].set_active(val);
    }    
    
// Link    
    if (!strcmp(path, "/link")) {
        int link_index = argv[0]->i - 1;
        int val = argv[1]->i;

        m_link[link_index].set_value(val);
    }   
    
// strip
    if (!strncmp(path, "/ch/gain", 8))          m_stripLayouts[SMP_INDEX(9)].m_fader.m_fader->set_value((float)OSC_STRIP_I0);
    
    if (!strncmp(path, "/ch/pan", 7))            m_stripLayouts[OSC_STRIP_INDEX1(8)].m_fader.m_Pan[0]->set_value(OSC_STRIP_I0 + 127);
//    if (!strncmp(path, "/ch/mute", 8))          m_stripLayouts[SMP_INDEX(9)].m_fader.m_MuteEnable->set_active(OSC_STRIP_B0);
    if (!strncmp(path, "/ch/solo", 8)) {
        if (OSC_STRIP_INDEX1(9) == m_solo_channel || m_solo_channel == -1) {
            m_stripLayouts[OSC_STRIP_INDEX1(9)].m_fader.m_SoloEnable->set_value(OSC_STRIP_B0);
            m_solo_channel = OSC_STRIP_B0 ? OSC_STRIP_INDEX1(9) : -1;
        }
    }
    if (!strncmp(path, "/ch/reset", 9))          m_stripLayouts[OSC_STRIP_INDEX1(10)].reset(alsa, OSC_STRIP_INDEX1(10));

}

#endif

void OMainWnd::on_control_changed(OOscControl* control) {
    OSC_STRIP_MSG2(control->get_osc_path(), control->get_value());
}

void OMainWnd::on_dsp_enable_changed(int n, const char* control_name) {

    if (block_events)
        return;
   

    if (!strcmp(control_name, CTL_NAME_CHANNEL_ACTIVE)) {
        set_dsp_channel(n, m_stripLayouts[n].m_DspEnable.get_value());
    }
}

void OMainWnd::set_dsp_channel(int n, bool enable) {
    if (enable) {
        if (m_dsp_channel != -1)
            m_stripLayouts[m_dsp_channel].m_DspEnable.set_value(false);
        m_dsp_channel = n;
        m_block_ui = true;
        m_dsp_layout.set_channel_type(m_stripLayouts[m_dsp_channel].get_channel_type());
        m_dsp_layout.set_view_type(HIDDEN);
        m_dsp_layout.set_ref_index(n, this);
        m_dsp_layout.set_view_type(SINGLE_DSP);
        m_dsp_layout.set_sensitive(true);

        m_block_ui = false;
    } else {
        m_dsp_channel = -1;
        m_dsp_layout.set_channel_type(MONO);
        m_dsp_layout.set_view_type(HIDDEN);
        m_dsp_layout.set_ref_index(16, this);
        m_dsp_layout.set_view_type(SINGLE_DSP);
        m_dsp_layout.set_sensitive(false);
    }
}

void OMainWnd::on_ch_lb_changed(int n) {
    char title[64];
    if (m_dsp_channel == n * 2) {
        m_stripLayouts[n * 2].m_DspEnable.set_value(false);
    } else if (m_dsp_channel == n * 2 + 1)
        m_stripLayouts[n * 2 + 1].m_DspEnable.set_value(false);

    if (m_link[n].get_value()) {

        m_stripLayouts[n * 2].set_channel_type(STEREO);

        m_stripLayouts[n * 2 + 1].set_view_type(VIEW_TYPE::HIDDEN);
        if (m_stripLayouts[n * 2 + 1].get_parent())
            m_grid.remove(m_stripLayouts[n * 2 + 1]);

        m_stripLayouts[n * 2].set_view_type(HIDDEN);
        m_stripLayouts[n * 2].set_view_type(m_view);

        snprintf(title, 64, "Ch %d-%d", n * 2 + 1, n * 2 + 2);
        m_stripLayouts[n * 2].m_title.set_label(title);

    } else {
        m_stripLayouts[n * 2].set_channel_type(MONO);
        m_stripLayouts[n * 2 + 1].set_channel_type(MONO);

        m_stripLayouts[n * 2].set_view_type(HIDDEN);
        m_stripLayouts[n * 2].set_view_type(m_view);

        if (!m_stripLayouts[n * 2 + 1].get_parent())
            m_grid.attach(m_stripLayouts[n * 2 + 1], n * 2 + 1, 2, 1, 1);

        m_stripLayouts[n * 2 + 1].set_view_type(HIDDEN);
        m_stripLayouts[n * 2 + 1].set_view_type(m_view);

        snprintf(title, 64, "Ch %d", n * 2 + 1);
        m_stripLayouts[n * 2].m_title.set_label(title);
    }
    resize(1, 1);
}

void OMainWnd::on_toggle_solo(int index) {
    if (m_SoloEnable[index].get_value()) {
        
        set_solo_channel(index);
    }
    else {
        release_solo_channel();
    }
}

void OMainWnd::set_solo_channel(int solo_channel) {
    for (int i = 0; i < NUM_CHANNELS; i++) {
        m_mute_store[i] = m_MuteEnable[i].get_value();
        if (i != solo_channel) {
            m_MuteEnable[i].set_value(true);
            usleep(RESET_VALUE_DELAY);
            m_SoloEnable[i].set_sensitive(false);
        }
        if (m_stripLayouts[i].get_channel_type() == STEREO ) {
            i++;
        }
    }
    m_solo_channel = solo_channel;
}

void OMainWnd::release_solo_channel() {
    for (int i = 0; i < NUM_CHANNELS; i++) {
        if (i != m_solo_channel) {
            m_MuteEnable[i].set_value(m_mute_store[i]);
            m_SoloEnable[i].set_sensitive(true);
        }
        usleep(RESET_VALUE_DELAY);
    }
    m_solo_channel = -1;
}

void OMainWnd::on_about_dialog_response(int response_id) {

    switch (response_id) {
        case Gtk::RESPONSE_CLOSE:
        case Gtk::RESPONSE_CANCEL:
        case Gtk::RESPONSE_DELETE_EVENT:
            m_Dialog.hide();
            break;
        default:
            break;
    }
}

OConfig* OMainWnd::GetConfig() {
    return &m_config;
}


void OMainWnd::add_osc_control(OOscControl* osd) {
    m_osc_control_map[osd->get_osc_path()] = osd;
}