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

#ifndef ODSPLAYOUT_H
#define ODSPLAYOUT_H

#include "OTypes.h"
#include "ODial.h"
#include "OComp.h"
#include "OEq.h"
#include "OFader.h"
#include "OMeter.h"
#include "ORoute.h"
#include "OAlsa.h"

class ODspLayout : public Gtk::VBox {
public:
    ODspLayout();
    ODspLayout(const ODspLayout& orig);
    virtual ~ODspLayout();
    
    void init(int index, OAlsa* alsa, Gtk::Window* wnd);

    void set_view_type(VIEW_TYPE i);
    
    void set_sensitive(bool val);

    void set_channel_type(CHANNEL_TYPE num_channels);
    CHANNEL_TYPE get_channel_type() {return m_channel_type;}
    
    void set_ref_index(int index, Gtk::Window* wnd);
    
    OComp m_comp;
    OEq m_eq;
    ORoute* m_route;
    
private:
    Gtk::Label m_filler;
    Gtk::Grid m_grid;
    CHANNEL_TYPE m_channel_type;
    Gtk::VSeparator m_eq_sep;
    
    Gtk::HSeparator m_sep;

};

#endif /* ODSPLAYOUT_H */

