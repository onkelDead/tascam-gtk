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

#include "OMaster.h"
#include "OMainWnd.h"

OMaster::OMaster() : Gtk::VBox() {
    //	set_size_request(120, -1);


    m_true_bypass.set_label("Mixer True\nBypass");
    m_true_bypass.set_name("comp-button");
    m_true_bypass.set_ledcolor(1., 1., 0., 1.);
    m_true_bypass.set_ledsize(6);
    m_true_bypass.set_fontsize(8);
    m_true_bypass.set_size_request(-1, 48);
//    m_true_bypass.set_align(Gtk::Align::ALIGN_CENTER);
    m_true_bypass.set_halign(Gtk::ALIGN_FILL);
    m_true_bypass.osc_init("/master/bypass");

    m_comp_to_stereo.set_label("Computer out\nto Stereo BUS");
    m_comp_to_stereo.set_name("comp-button");
    m_comp_to_stereo.set_ledcolor(1., 1., 0., 1.);
    m_comp_to_stereo.set_ledsize(6);
    m_comp_to_stereo.set_fontsize(8);
    m_comp_to_stereo.set_size_request(-1, 48);
//    m_comp_to_stereo.set_align(Gtk::Align::ALIGN_CENTER);
    m_comp_to_stereo.set_halign(Gtk::ALIGN_FILL);
    m_comp_to_stereo.osc_init("/master/busout");
    
    m_mute.set_label("Mute");
    m_mute.set_name("mute-button");
    m_mute.set_ledcolor(1., 0., 0., 1.);
    m_mute.set_ledsize(6);
    m_mute.set_fontsize(8);
    m_mute.set_halign(Gtk::ALIGN_FILL);
    m_mute.set_size_request(-1, 48);
    m_mute.osc_init("/master/mute");

    m_fader.set_name("fader");
    m_fader.set_range(0, 133);
    m_fader.set_inverted(true);
    m_fader.set_size_request(-1, 160);
    m_fader.set_draw_value(false);
    m_fader.set_increments(1, 5);
//    m_fader.add_mark(133, Gtk::PositionType::POS_RIGHT, "+6 dB");
//    m_fader.add_mark(123, Gtk::PositionType::POS_RIGHT, "+3 dB");
//    m_fader.add_mark(113, Gtk::PositionType::POS_RIGHT, "0 dB");
//    m_fader.add_mark(89, Gtk::PositionType::POS_RIGHT, "-10 dB");
//    m_fader.add_mark(73, Gtk::PositionType::POS_RIGHT, "-20 dB");
//    m_fader.add_mark(50, Gtk::PositionType::POS_RIGHT, "-40 dB");
//    m_fader.add_mark(34, Gtk::PositionType::POS_RIGHT, "-60 dB");
//    m_fader.add_mark(16, Gtk::PositionType::POS_RIGHT, "-90 dB");
//    m_fader.add_mark(0, Gtk::PositionType::POS_RIGHT, "-inf dB");
    m_fader.set_vexpand(true);


    m_meter_left.set_size_request(10, 160);
    m_meter_left.set_level_color(0, .7, 0, 1);
    m_meter_right.set_size_request(10, 160);
    m_meter_right.set_level_color(0, .7, 0, 1);
    m_meter_left.set_hexpand(false);
    m_meter_left.set_halign(Gtk::ALIGN_CENTER);
    m_meter_right.set_hexpand(false);
    m_meter_right.set_halign(Gtk::ALIGN_CENTER);


    add(m_grid);
}

OMaster::~OMaster() {
}

void OMaster::init(OAlsa* alsa, Gtk::Window* wnd) {
    char l_title[64];
    int val;

    OMainWnd* wnd_ = (OMainWnd*) wnd;

    m_route = &wnd_->m_routing;

    val = alsa->getInteger(CTL_MASTER, 0);
    m_fader.set_value(alsa->dBToSlider(val) + 1);
    m_fader.signal_value_changed().connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_control_changed), &m_fader));
    snprintf(l_title, sizeof (l_title), "%d dB", val - 127);
    m_fader.set_tooltip_text(l_title);
    m_fader.osc_init("/master/gain");
    wnd_->add_osc_control(&m_fader);
    m_fader.set_alsa_control_name(CTL_MASTER);

    m_true_bypass.set_value(alsa->getBoolean(CTL_NAME_BYPASS, 0) ? 1 : 0);
    m_true_bypass.signal_switched.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_control_changed), &m_true_bypass));

    m_comp_to_stereo.set_value(alsa->getBoolean(CTL_NAME_BUS_OUT, 0) ? 1 :0);
    m_comp_to_stereo.signal_switched.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_control_changed), &m_comp_to_stereo));

    m_mute.set_value(alsa->getBoolean(CTL_NAME_MASTER_MUTE, 0) ? 1 : 0);
    m_mute.signal_switched.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_control_changed), &m_mute));

    set_view_type(NORMAL);

}

void OMaster::reset(OAlsa* alsa) {

    m_fader.set_value(alsa->dBToSlider(127) + 1);
    usleep(RESET_VALUE_DELAY);

    alsa->setBoolean(CTL_NAME_BYPASS, 0, 0);
    m_true_bypass.set_value(alsa->getBoolean(CTL_NAME_BYPASS, 0) ? 1 : 0);
    usleep(RESET_VALUE_DELAY);

    alsa->setBoolean(CTL_NAME_BUS_OUT, 0, 0);
    m_comp_to_stereo.set_value(alsa->getBoolean(CTL_NAME_BUS_OUT, 0) ? 1 : 0);
    usleep(RESET_VALUE_DELAY);

    alsa->setBoolean(CTL_NAME_MASTER_MUTE, 0, 0);
    m_mute.set_value(alsa->getBoolean(CTL_NAME_MASTER_MUTE, 0) ? 1 : 0);
    usleep(RESET_VALUE_DELAY);
}

void OMaster::set_view_type(VIEW_TYPE view_type) {

    if (view_type == HIDDEN) {
        std::vector<Gtk::Widget*> childList = m_grid.get_children();
        std::vector<Gtk::Widget*>::iterator it;

        for (it = childList.begin(); it < childList.end(); it++) {
                m_grid.remove(**it);
        }
        m_route->set_view_type(view_type);
    }
    else {
        if (view_type == NORMAL) {
                m_grid.attach(*m_route, 0, 0, 3, 1);
        }
        m_grid.attach(m_true_bypass, 0, 8, 3, 1);
        m_grid.attach(m_comp_to_stereo, 0, 9, 3, 1);
        m_grid.attach(m_mute, 0, 10, 3, 1);
        m_grid.attach(m_fader, 0, 11, 1, 1);
        m_grid.attach(m_meter_left, 1, 11, 1, 1);
        m_grid.attach(m_meter_right, 2, 11, 1, 1);
        m_route->set_view_type(view_type);
    }
    m_view_type = view_type;
}
