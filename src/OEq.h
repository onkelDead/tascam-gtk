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

#include <lo/lo.h>

#include "OTypes.h"
#include "ODial.h"
#include "OAlsa.h"

#define EBLUE_NORMAL .5, .55, 1., 1.
#define EBLUE_LIGHT .78, .8, 1., 1.

extern const char *eq_low_freq_map[];
extern const int eq_low_freq_map_size;


extern const char *eq_high_freq_map[];
extern const int eq_high_freq_map_size;

char* eq_width_text(int val, char* buf, size_t buf_size);

char* eq_level_text(int val, char* buf, size_t buf_size);

char* eq_high_freq_text(int val, char* buf, size_t buf_size);

char* eq_lowhigh_freq_text(int val, char* buf, size_t buf_size);

char* eq_low_freq_text(int val, char* buf, size_t buf_size);

class OEq : public Gtk::VBox {
public:
    OEq();
    virtual ~OEq();
    
    void pack(VIEW_TYPE view_type, CHANNEL_TYPE channel_type);
    void unpack();


    void init(int index, OAlsa* alsa, Gtk::Window* wnd);
    
    void reset(OAlsa* alsa, int index);
 
    void save_values(FILE* file);    
    void load_values(Glib::ustring xml);

    int get_parameter_count() { return 10; }
    void get_parameter_decriptor(int parameter_index, lo_message reply);
    
    
    Gtk::ToggleButton* m_eq_enable;
    ODial* m_high_freq_gain;
    ODial* m_high_freq_band;
    ODial* m_mid_high_freq_gain;
    ODial* m_mid_high_freq_band;
    ODial* m_mid_high_freq_width;
    ODial* m_mid_low_freq_gain;
    ODial* m_mid_low_freq_band;
    ODial* m_mid_low_freq_width;
    ODial* m_low_freq_gain;
    ODial* m_low_freq_band;
    
    Gtk::Grid m_grid;
    int m_pack;
private:

};

#endif /* OEQ_H */

