/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OMeter.h
 * Author: onkel
 *
 * Created on January 3, 2017, 1:42 PM
 */

#ifndef OMETER_H
#define OMETER_H

#include <gtkmm/widget.h>

class OMeter : public Gtk::Widget {
public:
    OMeter();
    virtual ~OMeter();
    
    void set_level_direction(int direct); 
    void set_level_color(double red, double green, double blue, double alpha );
    void setLevel(int val);

protected:

    //Overrides:
    virtual void on_size_request(Gtk::Requisition* requisition) override;
    virtual void on_size_allocate(Gtk::Allocation& allocation) override;
    virtual void on_map() override;
    virtual void on_unmap() override;
    virtual void on_realize() override;
    virtual void on_unrealize() override;
    virtual bool on_expose_event(GdkEventExpose* event) override;

    Glib::RefPtr<Gdk::Window> m_refGdkWindow;
    
private:
    int direction;
    int level;
    int peak;
    int peak_count;
    double m_b_red, m_b_green, m_b_blue, m_b_alpha;
};

#endif /* OMETER_H */

