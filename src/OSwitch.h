/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OSwitch.h
 * Author: onkel
 *
 * Created on 24. Dezember 2024, 14:17
 */

#ifndef OSWITCH_H
#define OSWITCH_H

#include <gtkmm/widget.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/styleproperty.h>

#include "OTypes.h"

typedef char* (*value_callback)(int val, char* buf, size_t buf_size);

class OSwitch : public Gtk::Widget {
public:
    OSwitch();
    virtual ~OSwitch();
    
    virtual gboolean get_active() {
        return m_active;
    }
    virtual void set_active(gboolean new_val) {
        if (new_val != m_active) {
            m_active = new_val;
            queue_draw();
            signal_switched.emit();
        }
    }
    
    virtual void set_label(const char* label);

    virtual char* get_label() {
        return m_label;
    }
    
    virtual void set_fontsize(int fs);
    
    virtual void set_ledsize(int ls);

    virtual void set_value_callback(value_callback func) {
        m_value_callback = func;
    }
    
    virtual void set_ledcolor(double red, double green, double blue, double alpha);
    
    virtual void set_align(Gtk::Align align);
    
    sigc::signal<void> signal_switched;
    
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
    
    Glib::RefPtr<Gdk::Window> m_refGdkWindow;

private:
    value_callback m_value_callback;

    gboolean m_active;
    char *m_label;
    
    int m_fontsize;
    int m_textwidth;
    int m_textheight;
    int m_ledsize;
    
    Gtk::Align m_align = Gtk::Align::ALIGN_START;

    double m_b_red, m_b_green, m_b_blue, m_b_alpha;
    VIEW_TYPE m_view_type;
};

#endif /* OSWITCH_H */

