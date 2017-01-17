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

#include <lo/lo.h>

#include "OTypes.h"
#include "ODial.h"
#include "OMeter.h"
#include "OAlsa.h"
#include "OTypes.h"

#define CREAD_LIGHT 1., .8, .8, 1.
#define CREAD_NORMAL 1., .6, .6, 1.

extern const char *cp_ration_map[];

char* cp_threshold_text(int val, char* buf, size_t buf_size);

char* cp_gain_text(int val, char* buf, size_t buf_size);

char* cp_attack_text(int val, char* buf, size_t buf_size);

char* cp_release_text(int val, char* buf, size_t buf_size);

class OComp : public Gtk::VBox {
public:
    OComp();
    virtual ~OComp();
    
    void init(int index, OAlsa* alsa, Gtk::Window* wnd);
    void set_ref_index(int index, Gtk::Window* wnd_);

    void get_alsa_values(int index, OAlsa* alsa);
    
    void set_view_type(VIEW_TYPE view_type, CHANNEL_TYPE channel_type);
    
    void set_sensitive(bool val);
    bool is_active() { return m_enable->get_active(); }

    void reset(OAlsa* alsa, int index);

    void save_values(FILE* file);   
    void load_values(Glib::ustring xml);
    
    int get_parameter_count() { return 5; }
    void get_parameter_decriptor(int parameter_index, lo_message reply);
    
    Gtk::ToggleButton* m_enable;
    ODial* m_threshold;
    ODial* m_gain;
    ODial* m_attack;
    ODial* m_release;
    ODial* m_ratio;
    
    OMeter* m_reduction[2];
    
private:
    VIEW_TYPE m_view_type;
    Gtk::Grid m_grid;
};

#endif /* OCOMP_H */

