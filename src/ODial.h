/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ODial.h
 * Author: onkel
 *
 * Created on January 1, 2017, 5:43 PM
 */

#ifndef ODIAL_H
#define ODIAL_H

#include <gtkmm/widget.h>

typedef char* (*value_callback)(int val, char* buf);

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

    virtual gint get_scroll_step() {
        return m_scroll_step;
    }
    virtual void set_scroll_step(gint new_val);
    
    virtual void set_label(const char* label);
    virtual char* get_label() { return m_label; }

    void set_map(const char* map[]);
    void set_value_callback(value_callback func) { m_value_callback = func; }
    
    sigc::signal<void> signal_value_changed;

protected:

    //Overrides:
    virtual void on_size_request(Gtk::Requisition* requisition) override;
    virtual void on_size_allocate(Gtk::Allocation& allocation) override;
    virtual void on_map() override;
    virtual void on_unmap() override;
    virtual void on_realize() override;
    virtual void on_unrealize() override;
    virtual bool on_expose_event(GdkEventExpose* event) override;
    virtual bool on_button_press_event(GdkEventButton* event) override;
    virtual bool on_button_release_event(GdkEventButton* event) override;
    virtual bool on_motion_notify_event(GdkEventMotion* event) override;
    virtual bool on_scroll_event(GdkEventScroll* event) override;

    virtual void draw_text(const Cairo::RefPtr<Cairo::Context>& cr,
                       int rectangle_width, int rectangle_height, char* text);
    
    Glib::RefPtr<Gdk::Window> m_refGdkWindow;
    

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
};

#endif /* ODIAL_H */

