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

#include "OTypes.h"
#include "ODial.h"
#include "OComp.h"
#include "OEq.h"
#include "OFader.h"
#include "OMeter.h"
#include "OAlsa.h"


class OStripLayout : public Gtk::VBox {
public:
    OStripLayout();
    virtual ~OStripLayout();

    void init(int index, OAlsa* alsa, Gtk::Window* wnd);

    Gtk::Label m_title;
    Gtk::EventBox m_event_box;

    void reset(OAlsa* alsa, int index);
    
    void save_values(FILE* file, int indexnt = 0);
    
    void load_values(Glib::ustring xml);
    
    void set_view_type(VIEW_TYPE i);
    VIEW_TYPE get_view_type() {return m_view_type;}
    
    void set_channel_type(CHANNEL_TYPE num_channels);
    CHANNEL_TYPE get_channel_type() {return m_channel_type;}
    
    OComp m_comp;
    OEq m_eq;
    OFader m_fader;
    Gtk::ToggleButton m_DspEnable;
    
private:
    CHANNEL_TYPE m_channel_type;
    VIEW_TYPE m_view_type;
   
    
    Gtk::Grid m_grid;
    Gtk::HSeparator m_title_sep;
    Gtk::HSeparator m_comp_sep;
    Gtk::HSeparator m_eq_sep;
    Gtk::VSeparator m_sep;
};

#endif /* OSTRIPLAYOUT_H */

