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


#ifndef OSTRIPLAYOUT_H
#define OSTRIPLAYOUT_H

#include "OComp.h"
#include "ODial.h"
#include "OEq.h"
#include "OMeter.h"
#include "OAlsa.h"


class OStripLayout : public Gtk::VBox {
public:
    OStripLayout();
    virtual ~OStripLayout();

    void init(int index, OAlsa* alsa, Gtk::Window* wnd);

    OMeter m_meter;
    Gtk::Label m_title;
    Gtk::EventBox m_event_box;

    void reset(OAlsa* alsa, int index);
    
    void save_values(FILE* file, int indexnt = 0);
    
    void load_values(Glib::ustring xml);
    
    Gtk::ToggleButton m_MuteEnable;
    Gtk::ToggleButton m_SoloEnable;
    Gtk::VScale m_fader;
    ODial m_Pan;
    
    OEq m_eq;
    OComp m_comp;
    
private:
    Gtk::HBox m_hbox;
    Gtk::VBox m_box;

    Gtk::HBox m_fader_box;
    
    Gtk::VBox m_pvbox;
    Gtk::Label m_dB;
    
    Gtk::ToggleButton m_PhaseEnable;    
    Gtk::VSeparator m_sep;

    Gtk::HSeparator m_title_sep;

    Gtk::HBox m_panbox;
    Gtk::VBox m_pan_button_box;
    
    Gtk::HSeparator m_comp_sep;

    Gtk::HSeparator m_eq_sep;

};

#endif /* OSTRIPLAYOUT_H */

