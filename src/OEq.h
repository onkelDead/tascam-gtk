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


#ifndef OEQ_H
#define OEQ_H

#include "ODial.h"
#include "OAlsa.h"

class OEq : public Gtk::VBox {
public:
    OEq();
    virtual ~OEq();
    
    void init(int index, OAlsa* alsa);
    
    void reset(OAlsa* alsa, int index);
 
    void save_values(FILE* file);    
    void load_values(Glib::ustring xml);

private:
    Gtk::VBox m_box;

    Gtk::VBox l_eeb;
    Gtk::ToggleButton m_EqEnable;

    Gtk::HBox m_high_box;
    ODial m_high_freq_gain;
    ODial m_high_freq_band;
    
    Gtk::HBox m_mid_high_box;
    ODial m_mid_high_freq_gain;
    ODial m_mid_high_freq_band;
    Gtk::HBox m_mid_high_box1;
    ODial m_mid_high_freq_width;
    
    Gtk::HBox m_mid_low_box;
    ODial m_mid_low_freq_gain;
    ODial m_mid_low_freq_band;
    Gtk::HBox m_mid_low_box1;
    ODial m_mid_low_freq_width;
    
    Gtk::HBox m_low_box;
    ODial m_low_freq_gain;
    ODial m_low_freq_band;
    
};

#endif /* OEQ_H */

