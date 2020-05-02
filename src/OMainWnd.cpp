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
#include <libxml++-2.6/libxml++/libxml++.h>
#include <libxml++-2.6/libxml++/parsers/textreader.h>
#include <iostream>
#include <gtkmm-3.0/gtkmm/widget.h>

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

    GSettingsSchemaSource* source = g_settings_schema_source_get_default();
    if (g_settings_schema_source_lookup(source, TASCAMGTK_SCHEMA_ID, true)) {
        settings = Gio::Settings::create(TASCAMGTK_SCHEMA_ID);
        compact = settings->get_boolean("view-compact");
    }

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
    
    create_worker_threads(); 
    
    if (compact)
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
    m_refActionGroup->add(Gtk::Action::create("View", "_View"));
    m_refActionGroup->add(Gtk::Action::create("compact", Gtk::Stock::ZOOM_IN, "_Compact"),
            sigc::mem_fun(this, &OMainWnd::on_menu_view_compact));
    m_refActionGroup->add(Gtk::Action::create("normal", Gtk::Stock::ZOOM_OUT, "_Normal"),
            sigc::mem_fun(this, &OMainWnd::on_menu_view_normal));

    m_refUIManager = Gtk::UIManager::create();
    m_refUIManager->insert_action_group(m_refActionGroup);

    //Layout the actions in a menubar and toolbar:
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

            m_threshold[i].set_params(0, 32, 32, 1);
            m_threshold[i].set_label("Thresh");
            m_threshold[i].set_value_callback(cp_threshold_text);
            m_threshold[i].set_knob_background_color(CREAD_NORMAL);

            m_gain[i].set_params(0, 20, 0, 1);
            m_gain[i].set_label("Gain");
            m_gain[i].set_value_callback(cp_gain_text);
            m_gain[i].set_knob_background_color(CREAD_NORMAL);

            m_attack[i].set_params(0, 198, 0, 5);
            m_attack[i].set_label("Attack");
            m_attack[i].set_value_callback(cp_attack_text);
            m_attack[i].set_knob_background_color(CREAD_LIGHT);

            m_release[i].set_params(0, 99, 0, 1);
            m_release[i].set_label("Release");
            m_release[i].set_value_callback(cp_release_text);
            m_release[i].set_knob_background_color(CREAD_LIGHT);

            m_ratio[i].set_params(0, 14, 0, 1);
            m_ratio[i].set_label("Ratio");
            m_ratio[i].set_map(cp_ration_map);
            m_ratio[i].set_knob_background_color(CREAD_NORMAL);

            m_reduction[i].setLevel(32768);
            m_reduction[i].set_size_request(10, -1);
            m_reduction[i].set_level_direction(1);
            m_reduction[i].set_level_color(1, .6, .6, 1);
        }

        // equalizer controls
        {
            m_eq_enable[i].set_label("EQ");
            m_eq_enable[i].set_name("eq-button");
            m_eq_enable[i].set_vexpand(false);
            m_eq_enable[i].set_valign(Gtk::ALIGN_CENTER);

            m_high_freq_gain[i].set_label("High");
            m_high_freq_gain[i].set_value_callback(eq_level_text);
            m_high_freq_gain[i].set_params(0, 24, 12, 1);
            m_high_freq_gain[i].set_name("eq_high_gain");
            m_high_freq_gain[i].set_knob_background_color(EBLUE_NORMAL);

            m_high_freq_band[i].set_label("Freq");
            m_high_freq_band[i].set_value_callback(eq_high_freq_text);
            m_high_freq_band[i].set_params(0, 31, 15, 1);
            m_high_freq_band[i].set_knob_background_color(EBLUE_LIGHT);

            m_mid_high_freq_gain[i].set_label("Mid H");
            m_mid_high_freq_gain[i].set_params(0, 24, 12, 1);
            m_mid_high_freq_gain[i].set_value_callback(eq_level_text);
            m_mid_high_freq_gain[i].set_knob_background_color(EBLUE_NORMAL);

            m_mid_high_freq_band[i].set_label("Freq");
            m_mid_high_freq_band[i].set_params(0, 63, 27, 1);
            m_mid_high_freq_band[i].set_value_callback(eq_lowhigh_freq_text);
            m_mid_high_freq_band[i].set_knob_background_color(EBLUE_LIGHT);

            m_mid_high_freq_width[i].set_label("Width");
            m_mid_high_freq_width[i].set_value_callback(eq_width_text);
            m_mid_high_freq_width[i].set_params(0, 6, 2, 1);
            m_mid_high_freq_width[i].set_knob_background_color(EBLUE_LIGHT);
            m_mid_high_freq_width[i].set_hexpand(false);
            m_mid_high_freq_width[i].set_halign(Gtk::ALIGN_CENTER);

            m_mid_low_freq_gain[i].set_label("Mid L");
            m_mid_low_freq_gain[i].set_params(0, 24, 12, 1);
            m_mid_low_freq_gain[i].set_value_callback(eq_level_text);
            m_mid_low_freq_gain[i].set_knob_background_color(EBLUE_NORMAL);

            m_mid_low_freq_band[i].set_label("Freq");
            m_mid_low_freq_band[i].set_params(0, 63, 14, 1);
            m_mid_low_freq_band[i].set_value_callback(eq_lowhigh_freq_text);
            m_mid_low_freq_band[i].set_knob_background_color(EBLUE_LIGHT);

            m_mid_low_freq_width[i].set_label("Width");
            m_mid_low_freq_width[i].set_value_callback(eq_width_text);
            m_mid_low_freq_width[i].set_params(0, 6, 2, 1);
            m_mid_low_freq_width[i].set_knob_background_color(EBLUE_LIGHT);

            m_low_freq_gain[i].set_label("Low");
            m_low_freq_gain[i].set_params(0, 24, 12, 1);
            m_low_freq_gain[i].set_value_callback(eq_level_text);
            m_low_freq_gain[i].set_knob_background_color(EBLUE_NORMAL);

            m_low_freq_band[i].set_label("Freq");
            m_low_freq_band[i].set_params(0, 31, 5, 1);
            m_low_freq_band[i].set_value_callback(eq_low_freq_text);
            m_low_freq_band[i].set_knob_background_color(EBLUE_LIGHT);
        }

        if (i < NUM_CHANNELS) {
            m_Pan[i].set_params(0, 254, 127, 5);
            m_Pan[i].set_label("L Pan R");
            m_Pan[i].set_knob_background_color(1., .8, .3, 1.);


            m_MuteEnable[i].set_label("Mute");
            m_MuteEnable[i].set_name("mute-button");

            m_SoloEnable[i].set_label("Solo");
            m_SoloEnable[i].set_name("solo-button");

            m_PhaseEnable[i].set_label("Phase");
            m_PhaseEnable[i].set_name("phase-button");

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
        m_link[i].signal_toggled().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_ch_lb_changed), i));
        m_grid.attach(m_link[i], i * 2, 3, 2, 1);
    }

    // create DSP layout
    {
        m_dsp_layout.init(16, alsa, this);
        m_dsp_layout.set_view_type(PREPARE);
        m_dsp_layout.set_sensitive(false);
        //		m_grid.attach(m_dsp_layout, 0, 1, 16, 1);
    }
    
    m_route.init(alsa, this);
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
        m_Dialog.set_license("LGPL");

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
        });
    }
}

alsa_control* OMainWnd::get_alsa_widget(const char* info_name, int index, snd_ctl_elem_type_t t) {
    alsa_control* ac = 0;
    if (strcmp(info_name, "Master Mute Switch") == 0) {
        ac = new alsa_control;
        ac->type = ToggleButton;
        ac->tbwidget = &m_master.m_mute;
    }
    else if (strcmp(info_name, "Master Volume") == 0) {
        ac = new alsa_control;
        ac->type = Fader;
        ac->faderwidget = &m_master.m_fader;        
    }
    else if (strcmp(info_name, "DSP Bypass Switch") == 0) {
        ac = new alsa_control;
        ac->type = ToggleButton;
        ac->tbwidget = &m_master.m_true_bypass;
    }
    else if (strcmp(info_name, "Buss Out Switch") == 0) {
        ac = new alsa_control;
        ac->type = ToggleButton;
        ac->tbwidget = &m_master.m_comp_to_stereo;
    }
    else if (strcmp(info_name, "Line Out Route") == 0) {
        ac = new alsa_control;
        ac->type = ComboBox;
        ac->combo = &m_route.m_route[index];        
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
        ac->type = ToggleButton;
        ac->tbwidget = &m_comp_enable[index];
    }
    else if (strcmp(info_name, "Compressor Threshold Volume") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_threshold[index];
    }
    else if (strcmp(info_name, "EQ High Frequence") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_high_freq_band[index];
    }
    else if (strcmp(info_name, "EQ High Volume") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_high_freq_gain[index];
    }
    else if (strcmp(info_name, "EQ Low Frequence") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_low_freq_band[index];
    }
    else if (strcmp(info_name, "EQ Low Volume") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_low_freq_gain[index];
    }    
    else if (strcmp(info_name, "EQ MidHigh Frequence") == 0) {
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
    else if (strcmp(info_name, "EQ MidLow Frequence") == 0) {
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
        ac->type = ToggleButton;
        ac->tbwidget = &m_eq_enable[index];
    }
    else if (strcmp(info_name, "Mute Switch") == 0) {
        ac = new alsa_control;
        ac->type = ToggleButton;
        ac->tbwidget = &m_MuteEnable[index];
    }
    else if (strcmp(info_name, "Pan Left-Right Volume") == 0) {
        ac = new alsa_control;
        ac->type = Dial;
        ac->dial = &m_Pan[index];
    }
    else if (strcmp(info_name, "Phase Switch") == 0) {
        ac = new alsa_control;
        ac->type = ToggleButton;
        ac->tbwidget = &m_PhaseEnable[index];
    }
//    else {
//         printf("unknown control %s\n", info_name);
//    }
    return ac;
}

void OMainWnd::alsa_update_control(snd_hctl_elem_t *helem, int val) {
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
            case ToggleButton:
                widget->tbwidget->set_active(cv->value);
                break;
            case Fader:
                widget->faderwidget->set_value(alsa->dBToSlider(cv->value) + 1);
                break;    
            case ComboBox:
                widget->combo->set_active(cv->value);
                break;
            case Dial:
                widget->dial->set_value(cv->value);
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

        if (m_comp_enable[i].get_active())
            m_reduction[i].setLevel(alsa->sliderTodB(alsa->meters[i + 18] / 32768. * 133.) / 133. * 32768);
        else {
            m_reduction[i].setLevel(32767);
        }

        if (m_stripLayouts[i].get_channel_type() == STEREO) {
            ch_meter = alsa->sliderTodB(alsa->meters[i + 1] / 32768. * 133.) / 133. * 32768;
            m_stripLayouts[i].m_fader.m_meter[1].setLevel(ch_meter);
        }
#ifdef HAVE_OSC
        lo_message reply = lo_message_new();
        int ch_leds = m_stripLayouts[i].m_fader.m_meter[0].get_level() * 14 / 32768;
        int ch_led_mask = 1 << ch_leds;
        lo_message_add_int32(reply, i);
        lo_message_add_int32(reply, ch_led_mask - 1);
        m_Worker.send_osc_all("/strip/meter", reply);
        lo_message_free(reply);
#endif  
    }

    int left_level = alsa->sliderTodB(alsa->meters[16] / 32768. * 133.) / 133. * 32768;
    int right_level = alsa->sliderTodB(alsa->meters[17] / 32768. * 133.) / 133. * 32768;
    m_master.m_meter_left.setLevel(left_level);
    m_master.m_meter_right.setLevel(right_level);

    int master_leds = MAX(m_master.m_meter_left.get_level(), m_master.m_meter_right.get_level()) * 14 / 32768;
    int led_mask = 1 << master_leds;

#ifdef HAVE_OSC 
    lo_message reply = lo_message_new();
    lo_message_add_int32(reply, led_mask - 1);
    m_Worker.send_osc_all("/master/meter", reply);
    lo_message_free(reply);
#endif

}

void OMainWnd::on_menu_file_exit() {
    this->hide();
}

void OMainWnd::on_menu_file_about() {
    m_Dialog.show();
}

void OMainWnd::on_menu_file_reset() {
    m_master.reset(alsa);

    m_route.reset(alsa);

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
//    if (settings)
//        settings->set_boolean("view-compact", true);
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
//    if (settings)
//        settings->set_boolean("view-compact", false);

    alsa->on_active_button_control_changed(0, CTL_NAME_METER, &m_stripLayouts[0].m_DspEnable);

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
            if (!strcmp(reader.get_name().c_str(), "channel") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
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
            fprintf(file, "%d", (int) m_link[i].get_active());
            fprintf(file, "</link>\n");

        }

        fprintf(file, "\t<master>");
        fprintf(file, "%d", (int) m_master.m_fader.get_value());
        fprintf(file, "</master>\n");

        fprintf(file, "\t<mute>");
        fprintf(file, "%d", (int) m_master.m_mute.get_active());
        fprintf(file, "</mute>\n");

        fprintf(file, "\t<bypass>");
        fprintf(file, "%d", (int) m_master.m_true_bypass.get_active());
        fprintf(file, "</bypass>\n");

        fprintf(file, "\t<bus_out>");
        fprintf(file, "%d", (int) m_master.m_comp_to_stereo.get_active());
        fprintf(file, "</bus_out>\n");

        for (int i = 0; i < 8; i++) {

            fprintf(file, "\t<route index=\"%d\">", i);
            fprintf(file, "%d", (int) m_route.m_route[i].get_active_row_number());
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
            if (!strcmp(reader.get_name().c_str(), "link") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
                if (reader.has_attributes()) {
                    reader.move_to_first_attribute();
                    int index = atoi(reader.get_value().c_str());
                    reader.read();
                    m_link[index].set_active(atoi(reader.get_value().c_str()));
                    usleep(RESET_VALUE_DELAY);
                }
            }
            if (!strcmp(reader.get_name().c_str(), "master") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
                reader.read();
                m_master.m_fader.set_value(atoi(reader.get_value().c_str()));
                usleep(RESET_VALUE_DELAY);
            }
            if (!strcmp(reader.get_name().c_str(), "mute") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
                reader.read();
                m_master.m_mute.set_active(atoi(reader.get_value().c_str()) == 1);
                m_master.m_mute.toggled();
                usleep(RESET_VALUE_DELAY);
            }
            if (!strcmp(reader.get_name().c_str(), "bypass") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
                reader.read();
                m_master.m_true_bypass.set_active(atoi(reader.get_value().c_str()) == 1);
                m_master.m_true_bypass.toggled();
                usleep(RESET_VALUE_DELAY);
            }
            if (!strcmp(reader.get_name().c_str(), "bus_out") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
                reader.read();
                m_master.m_comp_to_stereo.set_active(atoi(reader.get_value().c_str()) == 1);
                m_master.m_comp_to_stereo.toggled();
                usleep(RESET_VALUE_DELAY);
            }
            if (!strcmp(reader.get_name().c_str(), "route") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
                if (reader.has_attributes()) {
                    reader.move_to_first_attribute();
                    int index = atoi(reader.get_value().c_str());
                    reader.read();
                    m_route.m_route[index].set_active(atoi(reader.get_value().c_str()));
                    usleep(RESET_VALUE_DELAY);
                }
            }
            if (!strcmp(reader.get_name().c_str(), "channel") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
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

    if (!strcmp(path, "/strip/list")) {
        for (int i = 0; i < NUM_CHANNELS; i++) {
            reply = lo_message_new();
            lo_message_add_string(reply, "AT");
            lo_message_add_string(reply, m_stripLayouts[i].m_title.get_label().c_str());
            lo_message_add_int32(reply, 1);
            lo_message_add_int32(reply, 1);
            lo_message_add_int32(reply, m_stripLayouts[i].m_fader.m_MuteEnable->get_active() ? 1 : 0);
            lo_message_add_int32(reply, m_stripLayouts[i].m_fader.m_SoloEnable->get_active() ? 1 : 0);
            lo_message_add_int32(reply, i);
            lo_message_add_int32(reply, (int32_t) 1);
            m_Worker.send_osc(client_index, "#reply", reply);
            lo_message_free(reply);
        }

        reply = lo_message_new();
        lo_message_add_string(reply, "M");
        lo_message_add_string(reply, "Master");
        lo_message_add_int32(reply, 1);
        lo_message_add_int32(reply, 1);
        lo_message_add_int32(reply, m_master.m_mute.get_active() ? 1 : 0);
        lo_message_add_int32(reply, 0);
        lo_message_add_int32(reply, 17);
        lo_message_add_int32(reply, 0);
        m_Worker.send_osc(client_index, "#reply", reply);
        lo_message_free(reply);

        reply = lo_message_new();
        lo_message_add_string(reply, "end_route_list");
        lo_message_add_int64(reply, 0);
        lo_message_add_int64(reply, 0);
        m_Worker.send_osc(client_index, "#reply", reply);
        lo_message_free(reply);

        reply = lo_message_new();
        lo_message_add_float(reply, m_master.m_fader.get_value() / 133.);
        m_Worker.send_osc_all("/master/fader", reply);
        lo_message_free(reply);
        for (int i = 0; i < NUM_CHANNELS; i++) {
            reply = lo_message_new();
            lo_message_add_int32(reply, i);
            lo_message_add_float(reply, m_stripLayouts[i].m_fader.m_fader->get_value() / 133.);
            m_Worker.send_osc_all("/strip/fader", reply);
            lo_message_free(reply);

            reply = lo_message_new();
            lo_message_add_int32(reply, i);
            lo_message_add_float(reply, m_stripLayouts[i].m_fader.m_Pan[0]->get_value() / 254.);
            m_Worker.send_osc_all("/strip/pan_stereo_position", reply);
            lo_message_free(reply);

        }
    }
    if (!strcmp(path, "/strip/fader")) {
        int channel_index = argv[0]->i;
        float val = argv[1]->f;

        m_stripLayouts[channel_index].m_fader.m_fader->set_value(133. * val);
    }
    if (!strcmp(path, "/master/fader")) {
        float val = argv[0]->f;

        m_master.m_fader.set_value(133. * val);
    }
    if (!strcmp(path, "/strip/pan_stereo_position")) {
        int channel_index = argv[0]->i;
        float val = argv[1]->f;

        m_stripLayouts[channel_index].m_fader.m_Pan[0]->set_value((int) (254 * val));
    }
    if (!strcmp(path, "/strip/mute")) {
        int channel_index = argv[0]->i;
        float val = argv[1]->f;
        m_stripLayouts[channel_index].m_fader.m_MuteEnable->set_active(val != 0);
    }
    if (!strcmp(path, "/master/mute")) {
        float val = argv[0]->f;
        m_master.m_mute.set_active(val != 0);
    }
    if (!strcmp(path, "/strip/solo")) {
        int channel_index = argv[0]->i;
        if (channel_index == m_solo_channel || m_solo_channel == -1) {
            float val = argv[1]->f;
            m_stripLayouts[channel_index].m_fader.m_SoloEnable->set_active(val != 0);
        }
    }
    if (!strcmp(path, "/strip/plugin/list")) {
        int channel_index = argv[0]->i;
        if (channel_index < NUM_CHANNELS) {

            lo_message reply = lo_message_new();

            lo_message_add_int32(reply, channel_index);
            lo_message_add_int32(reply, 1);
            lo_message_add_string(reply, "Compressor");
            lo_message_add_int32(reply, m_comp_enable[channel_index].get_active() ? 1 : 0);

            lo_message_add_int32(reply, 2);
            lo_message_add_string(reply, "Equalizer");
            lo_message_add_int32(reply, m_eq_enable[channel_index].get_active() ? 1 : 0);

            m_Worker.send_osc(client_index, "/strip/plugin/list", reply);
            lo_message_free(reply);
        }
    }
    if (!strcmp(path, "/strip/plugin/descriptor")) {
        int channel_index = argv[0]->i32;
        int plugin_index = argv[1]->i32;
        lo_message reply;
        int flags;
        if (channel_index < NUM_CHANNELS) {

            OStripLayout* sl = &m_stripLayouts[channel_index];

            if (plugin_index == 1) { // compressor
                for (int i = 0; i < sl->m_comp.get_parameter_count(); i++) {
                    reply = lo_message_new();
                    lo_message_add_int32(reply, channel_index); // channel index
                    lo_message_add_int32(reply, plugin_index); // plugin index
                    sl->m_comp.get_parameter_decriptor(i, reply);
                    m_Worker.send_osc(client_index, "/strip/plugin/descriptor", reply);
                    lo_message_free(reply);
                }
            }
            if (plugin_index == 2) { // equalizer
                for (int i = 0; i < sl->m_eq.get_parameter_count(); i++) {
                    reply = lo_message_new();
                    lo_message_add_int32(reply, channel_index); // channel index
                    lo_message_add_int32(reply, plugin_index); // plugin index
                    sl->m_eq.get_parameter_decriptor(i, reply);
                    m_Worker.send_osc(client_index, "/strip/plugin/descriptor", reply);
                    lo_message_free(reply);
                }
            }

            reply = lo_message_new();
            lo_message_add_int32(reply, channel_index);
            lo_message_add_int32(reply, plugin_index);
            m_Worker.send_osc(client_index, "/strip/plugin/descriptor_end", reply);
            lo_message_free(reply);

        }
    }
    if (!strcmp(path, "/strip/plugin/reset")) {
        int channel_index = argv[0]->i32;
        int plugin_index = argv[1]->i32;
        if (plugin_index == 1) { // compressor
            m_stripLayouts[channel_index].m_comp.reset(alsa, channel_index);
        }
        if (plugin_index == 2) { // equalizer
            m_stripLayouts[channel_index].m_eq.reset(alsa, channel_index);
        }
    }
    if (!strcmp(path, "/strip/plugin/activate")) {
        int channel_index = argv[0]->i32;
        int plugin_index = argv[1]->i32;
        if (plugin_index == 1) { // compressor
            m_comp_enable[channel_index].set_active(true);
        }
        if (plugin_index == 2) { // equalizer
            m_eq_enable[channel_index].set_active(true);
        }
    }
    if (!strcmp(path, "/strip/plugin/deactivate")) {
        int channel_index = argv[0]->i32;
        int plugin_index = argv[1]->i32;
        if (plugin_index == 1) { // compressor
            m_comp_enable[channel_index].set_active(false);
        }
        if (plugin_index == 2) { // equalizer
            m_eq_enable[channel_index].set_active(false);
        }
    }
    if (!strcmp(path, "/strip/plugin/parameter")) {
        int channel_index = argv[0]->i32;
        int plugin_index = argv[1]->i32;
        int parameter_index = argv[2]->i32;
        if (plugin_index == 1) { // compressor
            switch (parameter_index) {
                case 1: // threshold
                    m_threshold[channel_index].set_value((int) argv[3]->f + 32);
                    break;
                case 2: // gain
                    m_gain[channel_index].set_value((int) argv[3]->f);
                    break;
                case 3: // attack
                    m_attack[channel_index].set_value((int) argv[3]->f - 2);
                    break;
                case 4: // release
                    m_release[channel_index].set_value((int) argv[3]->f - 1);
                    break;
                case 5: // ratio
                    m_ratio[channel_index].set_value((int) argv[3]->f);
                    break;
            }
        }
        if (plugin_index == 2) { // equalizer
            switch (parameter_index) {
                case 1: // high gain
                    m_high_freq_gain[channel_index].set_value((int) argv[3]->f + 12);
                    break;
                case 2: // high freq
                    m_high_freq_band[channel_index].set_value((int) argv[3]->f);
                    break;
                case 3: // mid high gain
                    m_mid_high_freq_gain[channel_index].set_value((int) argv[3]->f + 12);
                    break;
                case 4: // mid high freq
                    m_mid_high_freq_band[channel_index].set_value((int) argv[3]->f);
                    break;
                case 5: // mid high Q
                    m_mid_high_freq_width[channel_index].set_value((int) argv[3]->f);
                    break;
                case 6: // mid low gain
                    m_mid_low_freq_gain[channel_index].set_value((int) argv[3]->f + 12);
                    break;
                case 7: // mid low freq
                    m_mid_low_freq_band[channel_index].set_value((int) argv[3]->f);
                    break;
                case 8: // mid low Q
                    m_mid_low_freq_width[channel_index].set_value((int) argv[3]->f);
                    break;
                case 9: // low gain
                    m_low_freq_gain[channel_index].set_value((int) argv[3]->f + 12);
                    break;
                case 10: // low freq
                    m_low_freq_band[channel_index].set_value((int) argv[3]->f);
                    break;
            }
        }
    }
}
#endif

void OMainWnd::on_ch_fader_changed(int n, const char* control_name, Gtk::VScale* control, Gtk::Label * label_) {
 
    if (block_events)
        return;   
    
#ifdef HAVE_OSC  
    lo_message reply = lo_message_new();
#endif  

    if (!strcmp(control_name, CTL_NAME_FADER)) {
#ifdef HAVE_OSC  
        lo_message_add_int32(reply, n);
        lo_message_add_float(reply, control->get_value() / 133.);
        m_Worker.send_osc_all("/strip/fader", reply);
#endif  
        if (m_stripLayouts[n].get_channel_type() == STEREO) {
            m_fader[n + 1].set_value(m_fader[n].get_value());
        }
    }
#ifdef HAVE_OSC  
    if (!strcmp(control_name, CTL_MASTER)) {
        lo_message_add_float(reply, control->get_value() / 133.);
        m_Worker.send_osc_all("/master/fader", reply);
    }
    lo_message_free(reply);
#endif  

    
    alsa->on_range_control_changed(n, control_name, control, label_);
}

void OMainWnd::on_ch_dial_changed(int n, const char* control_name) {
    int org_index = -1;
    if (m_block_ui)
        return;

    if (n == 16) { // singel dsp signal
        org_index = n;

        if (m_dsp_channel != -1)
            n = m_dsp_channel;
        else
            return;
    }

    if (!strcmp(control_name, CTL_NAME_PAN)) {
#ifdef HAVE_OSC  
        lo_message reply = lo_message_new();
        lo_message_add_int32(reply, n);
        lo_message_add_float(reply, m_Pan[n].get_value() / 254.);
        m_Worker.send_osc_all("/strip/pan_stereo_position", reply);
        lo_message_free(reply);
#endif
        alsa->on_dial_control_changed(n, control_name, &m_Pan[n]);
    }
    // compressor dial changed
    {
        if (!strcmp(control_name, CTL_NAME_CP_THRESHOLD)) {
            alsa->on_dial_control_changed(n, control_name, &m_threshold[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_threshold[n + 1].set_value(m_threshold[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_CP_GAIN)) {
            alsa->on_dial_control_changed(n, control_name, &m_gain[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_gain[n + 1].set_value(m_gain[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_CP_ATTACK)) {
            alsa->on_dial_control_changed(n, control_name, &m_attack[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_attack[n + 1].set_value(m_attack[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_CP_RELEASE)) {
            alsa->on_dial_control_changed(n, control_name, &m_release[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_release[n + 1].set_value(m_release[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_CP_RATIO)) {
            alsa->on_dial_control_changed(n, control_name, &m_ratio[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_ratio[n + 1].set_value(m_ratio[n].get_value());
            }
        }
    }

    // equalizer dial changed
    {
        if (!strcmp(control_name, CTL_NAME_EQ_HIGH_FREQ)) {
            alsa->on_dial_control_changed(n, control_name, &m_high_freq_band[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_high_freq_band[n + 1].set_value(m_high_freq_band[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_EQ_HIGH_LEVEL)) {
            alsa->on_dial_control_changed(n, control_name, &m_high_freq_gain[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_high_freq_gain[n + 1].set_value(m_high_freq_gain[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_EQ_MIDHIGH_FREQ)) {
            alsa->on_dial_control_changed(n, control_name, &m_mid_high_freq_band[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_mid_high_freq_band[n + 1].set_value(m_mid_high_freq_band[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_EQ_MIDHIGH_LEVEL)) {
            alsa->on_dial_control_changed(n, control_name, &m_mid_high_freq_gain[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_mid_high_freq_gain[n + 1].set_value(m_mid_high_freq_gain[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_EQ_MIDHIGHWIDTH_FREQ)) {
            alsa->on_dial_control_changed(n, control_name, &m_mid_high_freq_width[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_mid_high_freq_width[n + 1].set_value(m_mid_high_freq_width[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_EQ_MIDLOW_FREQ)) {
            alsa->on_dial_control_changed(n, control_name, &m_mid_low_freq_band[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_mid_low_freq_band[n + 1].set_value(m_mid_low_freq_band[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_EQ_MIDLOW_LEVEL)) {
            alsa->on_dial_control_changed(n, control_name, &m_mid_low_freq_gain[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_mid_low_freq_gain[n + 1].set_value(m_mid_low_freq_gain[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_EQ_MIDLOWWIDTH_FREQ)) {
            alsa->on_dial_control_changed(n, control_name, &m_mid_low_freq_width[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_mid_low_freq_width[n + 1].set_value(m_mid_low_freq_width[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_EQ_LOW_FREQ)) {
            alsa->on_dial_control_changed(n, control_name, &m_low_freq_band[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_low_freq_band[n + 1].set_value(m_low_freq_band[n].get_value());
            }
        }
        if (!strcmp(control_name, CTL_NAME_EQ_LOW_LEVEL)) {
            alsa->on_dial_control_changed(n, control_name, &m_low_freq_gain[n]);
            if (m_stripLayouts[n].get_channel_type() == STEREO) {
                usleep(RESET_VALUE_DELAY);
                m_low_freq_gain[n + 1].set_value(m_low_freq_gain[n].get_value());
            }
        }
    }
}

void OMainWnd::on_ch_tb_changed(int n, const char* control_name) {

    if (block_events)
        return;
    
    if (!strcmp(control_name, CTL_NAME_CP_ENABLE)) {
        alsa->on_toggle_button_control_changed(n, control_name, &m_comp_enable[n]);
        if (m_stripLayouts[n].get_channel_type() == STEREO) {
            usleep(RESET_VALUE_DELAY);
            m_comp_enable[n + 1].set_active(m_comp_enable[n].get_active());
        }
    }

    if (!strcmp(control_name, CTL_NAME_EQ_ENABLE)) {
        alsa->on_toggle_button_control_changed(n, control_name, &m_eq_enable[n]);
        if (m_stripLayouts[n].get_channel_type() == STEREO) {
            usleep(RESET_VALUE_DELAY);
            m_eq_enable[n + 1].set_active(m_eq_enable[n].get_active());
        }
    }

    if (!strcmp(control_name, CTL_NAME_MUTE)) {
#ifdef HAVE_OSC  
        lo_message reply = lo_message_new();
        lo_message_add_int32(reply, n);
        lo_message_add_float(reply, m_MuteEnable[n].get_active() ? 1. : 0.);
        m_Worker.send_osc_all("/strip/mute", reply);
        lo_message_free(reply);
#endif  
        alsa->on_toggle_button_control_changed(n, control_name, &m_MuteEnable[n]);
        if (m_stripLayouts[n].get_channel_type() == STEREO) {
            usleep(RESET_VALUE_DELAY);
            m_MuteEnable[n + 1].set_active(m_MuteEnable[n].get_active());
        }
    }
    if (!strcmp(control_name, CTL_NAME_SOLO)) {
        if (m_solo_channel == n || m_solo_channel == -1) {
#ifdef HAVE_OSC  
            lo_message reply = lo_message_new();
            lo_message_add_int32(reply, n);
            lo_message_add_float(reply, m_SoloEnable[n].get_active() ? 1. : 0.);
            m_Worker.send_osc_all("/strip/solo", reply);
            lo_message_free(reply);
#endif
            if (m_SoloEnable[n].get_active())
                set_solo_channel(n);
            else
                release_solo_channel();
        }
    }
    if (!strcmp(control_name, CTL_NAME_PHASE)) {
#ifdef HAVE_OSC  
        lo_message reply = lo_message_new();
        lo_message_add_int32(reply, n);
        lo_message_add_float(reply, m_PhaseEnable[n].get_active() ? 1. : 0.);
        m_Worker.send_osc_all("/strip/solo", reply);
        lo_message_free(reply);
#endif  
        alsa->on_toggle_button_control_changed(n, control_name, &m_PhaseEnable[n]);
    }
    if (!strcmp(control_name, CTL_NAME_MASTER_MUTE)) {
#ifdef HAVE_OSC  
        lo_message reply = lo_message_new();
        lo_message_add_float(reply, m_master.m_mute.get_active() ? 1. : 0.);
        m_Worker.send_osc_all("/master/mute", reply);
        lo_message_free(reply);
#endif  
        alsa->on_toggle_button_control_changed(n, control_name, &m_master.m_mute);
    }
    if (!strcmp(control_name, CTL_NAME_BYPASS)) {
#ifdef HAVE_OSC  
        lo_message reply = lo_message_new();
        lo_message_add_float(reply, m_master.m_true_bypass.get_active() ? 1. : 0.);
        m_Worker.send_osc_all("/master/bypass", reply);
        lo_message_free(reply);
#endif  
        alsa->on_toggle_button_control_changed(n, control_name, &m_master.m_true_bypass);
    }

    if (!strcmp(control_name, CTL_NAME_BUS_OUT)) {
#ifdef HAVE_OSC  
        lo_message reply = lo_message_new();
        lo_message_add_float(reply, m_master.m_comp_to_stereo.get_active() ? 1. : 0.);
        m_Worker.send_osc_all("/master/bussout", reply);
        lo_message_free(reply);
#endif  
        alsa->on_toggle_button_control_changed(n, control_name, &m_master.m_comp_to_stereo);
    }

    if (!strcmp(control_name, CTL_NAME_CHANNEL_ACTIVE)) {
        set_dsp_channel(n, m_stripLayouts[n].m_DspEnable.get_active());
        if (m_stripLayouts[n].get_channel_type() == STEREO) {
            alsa->on_active_button_control_changed(n + 32, CTL_NAME_METER, &m_stripLayouts[n].m_DspEnable);
        } else
            alsa->on_active_button_control_changed(n, CTL_NAME_METER, &m_stripLayouts[n].m_DspEnable);
    }

}

void OMainWnd::set_dsp_channel(int n, bool enable) {
    if (enable) {
        if (m_dsp_channel != -1)
            m_stripLayouts[m_dsp_channel].m_DspEnable.set_active(false);
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
        m_stripLayouts[n * 2].m_DspEnable.set_active(false);
    } else if (m_dsp_channel == n * 2 + 1)
        m_stripLayouts[n * 2 + 1].m_DspEnable.set_active(false);

    if (m_link[n].get_active()) {

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

void OMainWnd::set_solo_channel(int solo_channel) {
    for (int i = 0; i < NUM_CHANNELS; i++) {
        m_mute_store[i] = m_MuteEnable[i].get_active();
        if (i != solo_channel) {
            m_MuteEnable[i].set_active(true);
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
            m_MuteEnable[i].set_active(m_mute_store[i]);
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
