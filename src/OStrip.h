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

#ifndef OSTRIP_H
#define OSTRIP_H

#include <iostream>

#include "OTypes.h"
#include "OMeter.h"
#include "ODial.h"
#include "OSwitch.h"
#include "OAlsa.h"

class OStrip : public Gtk::VBox {
public:
    OStrip();
    OStrip(const OStrip& orig);
    virtual ~OStrip();

    void init(int index, OAlsa* alsa, Gtk::Window* wnd);
    
    void set_view_type(VIEW_TYPE view_type, CHANNEL_TYPE channel_type);
    
    void reset(OAlsa* alsa, int index);
    void save_values(FILE* file);    
    void load_values(Glib::ustring xml);    
    char* get_alsa_name() { return 0; }
    
    void on_fader_changed(OOscControl*);
    
    OSwitch* m_MuteEnable;
    OSwitch* m_SoloEnable;
    OSwitch* m_PhaseEnable[2];    
    OFader* m_fader;
    ODial* m_Pan[2];
    
    OMeter m_meter[2];
    Gtk::Label m_dB;

    
private:
    VIEW_TYPE m_view_type;
    Gtk::Grid m_grid;
};

#endif /* OSTRIP_H */
