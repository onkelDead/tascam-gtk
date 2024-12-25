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


#ifndef OOSCDIALOG_H
#define OOSCDIALOG_H

#include <gtkmm.h>

#include "OConfig.h"

class OOscDialog : public Gtk::Dialog {
public:
    OOscDialog();
    virtual ~OOscDialog();

    void on_btn_cancel_clicked();
    void on_btn_ok_clicked();

    void SetData(OConfig* config);
    void GetData(OConfig* config);
    
    bool GetResult();
    
protected:
    Glib::RefPtr<Gtk::CssProvider> m_refCssProvider;
            
private:

    bool m_result;

    Gtk::Grid m_grid;
    Gtk::Label m_lbl_port;
    Gtk::Entry m_osc_port;
    Gtk::Label m_lbl_no_meters;
    Gtk::CheckButton m_chk_no_meters;
    Gtk::Label m_lbl_full_update;
    Gtk::CheckButton m_chk_full_update;    
};

#endif /* OOSCDIALOG_H */

