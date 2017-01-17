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


#ifndef OMASTER_H
#define OMASTER_H

#include "OTypes.h"
#include "ODial.h"
#include "OAlsa.h"
#include "OMeter.h"


class OMaster : public Gtk::VBox  {
public:
    OMaster();
    virtual ~OMaster();
    
    void init(OAlsa* alsa, Gtk::Window* wnd);    
    
    void set_view_type(VIEW_TYPE pack);
    
    void reset(OAlsa* alsa);
    
    OMeter m_meter_left;
    OMeter m_meter_right;
    
    Gtk::VScale m_fader;
    Gtk::ToggleButton m_mute;
    Gtk::ToggleButton m_true_bypass;
    Gtk::ToggleButton m_comp_to_stereo;
    Gtk::ComboBoxText m_route[8];
    
private:
    VIEW_TYPE m_view_type;
    Gtk::Grid m_grid;
};

#endif /* OMASTER_H */

