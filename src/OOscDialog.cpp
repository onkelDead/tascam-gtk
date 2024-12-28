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

#include <iostream>
#include "OOscDialog.h"

OOscDialog::OOscDialog() : Gtk::Dialog() {
    set_name("OOscDialog");
    set_title("OSC settings");
    set_modal(true);
    set_default_size(400, 200);
    
//    m_refCssProvider = Gtk::CssProvider::create();
//    auto refStyleContext = get_style_context();
//    refStyleContext->add_provider(m_refCssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
//
//    try {
//        if (Glib::file_test(PKGDATADIR "/tascam-gtk.css", Glib::FILE_TEST_EXISTS))
//            m_refCssProvider->load_from_path(PKGDATADIR "/tascam-gtk.css");
//        else
//            m_refCssProvider->load_from_path("./data/tascam-gtk.css");
//    } catch (const Gtk::CssProviderError& ex) {
//        std::cerr << "CssProviderError, Gtk::CssProvider::load_from_path() failed: "
//                << ex.what() << std::endl;
//    } catch (const Glib::Error& ex) {
//        std::cerr << "Error, Gtk::CssProvider::load_from_path() failed: "
//                << ex.what() << std::endl;
//    }
//
//    auto screen = Gdk::Screen::get_default();
//    refStyleContext->add_provider_for_screen(Gdk::Screen::get_default(), m_refCssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);   

    m_chk_no_meters.set_ledcolor(1.,1.,0.,1.);
    m_chk_no_meters.set_align(Gtk::Align::ALIGN_CENTER);
    
    m_chk_full_update.set_ledcolor(0.,1.,0.,1.);
    m_chk_full_update.set_align(Gtk::Align::ALIGN_CENTER);
    
       m_chk_log_osc.set_ledcolor(1.,.3,0.,1.);
    m_chk_log_osc.set_align(Gtk::Align::ALIGN_CENTER);
    
    m_lbl_port.set_text("OSC port");
    m_lbl_port.set_hexpand(true);
    m_grid.attach(m_lbl_port, 0, 0, 1, 1);
    m_grid.attach(m_osc_port, 1, 0, 1, 1);

    m_lbl_no_meters.set_text("OSC no meters");
    m_lbl_no_meters.set_hexpand(true);
    m_grid.attach(m_lbl_no_meters, 0, 1, 1, 1);
    m_grid.attach(m_chk_no_meters, 1, 1, 1, 1);

    m_lbl_full_update.set_text("OSC full update new client");
    m_lbl_full_update.set_hexpand(true);
    m_grid.attach(m_lbl_full_update, 0, 2, 1, 1);
    m_grid.attach(m_chk_full_update, 1, 2, 1, 1);    

    m_lbl_log_osc.set_text("Log all osc traffic");
    m_lbl_log_osc.set_hexpand(true);
    m_grid.attach(m_lbl_log_osc, 0, 3, 1, 1);
    m_grid.attach(m_chk_log_osc, 1, 3, 1, 1);    
    
    m_grid.set_halign(Gtk::ALIGN_FILL);
    m_grid.set_valign(Gtk::ALIGN_FILL);
    m_grid.set_hexpand(true);
    
    get_content_area()->add(m_grid);
    
    add_button(Gtk::Stock::OK, Gtk::ResponseType::RESPONSE_OK)
            ->signal_clicked().connect(sigc::mem_fun(*this, &OOscDialog::on_btn_ok_clicked));
    add_button(Gtk::Stock::CANCEL, Gtk::ResponseType::RESPONSE_CANCEL)
            ->signal_clicked().connect(sigc::mem_fun(*this, &OOscDialog::on_btn_cancel_clicked));
    
    set_halign(Gtk::ALIGN_FILL);
    set_valign(Gtk::ALIGN_FILL);
    set_hexpand(true);

    show_all_children(true);
    queue_draw();
}


OOscDialog::~OOscDialog() {
}

void OOscDialog::on_btn_cancel_clicked() {
    m_result = false;
    hide();
}

void OOscDialog::on_btn_ok_clicked() {
    m_result = true;
    hide();
}

bool OOscDialog::GetResult() {
    return m_result;
}

void OOscDialog::SetData(OConfig* config) {
    m_osc_port.set_text(config->get_string(SETTINGS_OSC_PORT));
    m_chk_no_meters.set_active(config->get_boolean(SETTINGS_OSC_NO_METERS));
    m_chk_full_update.set_active(config->get_boolean(SETTINGS_OSC_CLIENT_FULL_UPDATE));
    m_chk_log_osc.set_active(config->get_boolean(SETTINGS_OSC_LOG_ALL));
}

void OOscDialog::GetData(OConfig* config) {
    config->set_string(SETTINGS_OSC_PORT, m_osc_port.get_text().c_str());
    config->set_boolean(SETTINGS_OSC_NO_METERS, m_chk_no_meters.get_active());
    config->set_boolean(SETTINGS_OSC_CLIENT_FULL_UPDATE, m_chk_full_update.get_active());
    config->set_boolean(SETTINGS_OSC_LOG_ALL, m_chk_log_osc.get_active());
}