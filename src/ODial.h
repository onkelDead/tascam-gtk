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

#ifndef ODIAL_H
#define ODIAL_H

#include <gtkmm/widget.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/styleproperty.h>

#include "OTypes.h"

typedef char* (*value_callback)(int val, char* buf, size_t buf_size);

class ODial : public Gtk::Widget {
public:
    ODial();
    virtual ~ODial();

    virtual void set_min(int m) {
        m_min = m;
    }

    virtual void set_max(int m) {
        m_max = m;
    }

    virtual void set_params(int minimum, int maximum, int default_value, int scroll_step_size);

    virtual gint get_value() {
        return m_value;
    }
    virtual void set_value(gint new_val);
    virtual void set_default(gint new_val);

    virtual gint get_default() {
        return m_default;
    }

    virtual gint get_scroll_step() {
        return m_scroll_step;
    }
    virtual void set_scroll_step(gint new_val);

    virtual void set_label(const char* label);

    virtual char* get_label() {
        return m_label;
    }

    virtual void set_map(const char* map[]);

    virtual void set_value_callback(value_callback func) {
        m_value_callback = func;
    }

    void set_view_type(VIEW_TYPE type);
    
    virtual void set_knob_background_color(double red, double green, double blue, double alpha);

    sigc::signal<void> signal_value_changed;

    virtual void reset();

protected:

    //Overrides:
    Gtk::SizeRequestMode get_request_mode_vfunc() const override;
    void get_preferred_width_vfunc(int& minimum_width, int& natural_width) const override;
    void get_preferred_height_for_width_vfunc(int width, int& minimum_height, int& natural_height) const override;
    void get_preferred_height_vfunc(int& minimum_height, int& natural_height) const override;
    void get_preferred_width_for_height_vfunc(int height, int& minimum_width, int& natural_width) const override;
    virtual void on_size_allocate(Gtk::Allocation& allocation) override;
    virtual void on_map() override;
    virtual void on_unmap() override;
    virtual void on_realize() override;
    virtual void on_unrealize() override;
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    virtual bool on_button_press_event(GdkEventButton* event) override;
    virtual bool on_button_release_event(GdkEventButton* event) override;
    virtual bool on_motion_notify_event(GdkEventMotion* event) override;
    virtual bool on_scroll_event(GdkEventScroll* event) override;

    virtual void draw_text(const Cairo::RefPtr<Cairo::Context>& cr,
            int rectangle_width, int rectangle_height, char* text);
    //
    //    void on_parsing_error(const Glib::RefPtr<const Gtk::CssSection>& section, const Glib::Error& error);
    //
    Glib::RefPtr<Gdk::Window> m_refGdkWindow;
    //
    //    Glib::RefPtr<Gtk::CssProvider> m_refCssProvider;

    //    int m_scale;

private:
    value_callback m_value_callback;

    char *m_label;
    const char** m_map;
    gint m_value;
    gint m_last_y;
    gboolean m_in_motion;

    gint m_default;
    gint m_min;
    gint m_max;

    gint m_scroll_step;

    double m_b_red, m_b_green, m_b_blue, m_b_alpha;
    VIEW_TYPE m_view_type;
};

#endif /* ODIAL_H */

