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

#ifndef OROUTING_H
#define OROUTING_H

#include "OTypes.h"
#include "ODial.h"
#include "OSwitch.h"
#include "ORoute.h"
#include "OFader.h"
#include "OAlsa.h"


class ORouting : public Gtk::VBox {
public:
    ORouting();
    virtual ~ORouting();

    void init(OAlsa* alsa, Gtk::Window* wnd);    

    void set_view_type(VIEW_TYPE pack);
    
    void reset(OAlsa* alsa);
    
    ORoute m_route[8];
    
private:
    Gtk::Label m_label;
    Gtk::Grid m_grid;

};

#endif /* OROUTING_H */

