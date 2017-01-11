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

#ifndef OCOMP_H
#define OCOMP_H

#include "ODial.h"
#include "OMeter.h"
#include "OAlsa.h"

class OComp : public Gtk::VBox {
public:
    OComp();
    virtual ~OComp();
    
    void init(int index, OAlsa* alsa);
    
    OMeter m_reduction;
    
    bool is_active() { return m_CompEnable.get_active(); }

    void reset(OAlsa* alsa, int index);

    void save_values(FILE* file);   
    void load_values(Glib::ustring xml);
    
private:
    Gtk::VBox m_box;

    Gtk::HBox m_tg_box;    
    ODial m_threshold;
    ODial m_gain;
    
    Gtk::HBox m_ar_box;
    ODial m_attack;
    ODial m_release;
    
    Gtk::HBox m_r_box;
    
    Gtk::VBox m_re_box;
    ODial m_ratio;
    
    Gtk::VBox l_evb;
    Gtk::ToggleButton m_CompEnable;
    
    Gtk::HBox m_red_box;

};

#endif /* OCOMP_H */

