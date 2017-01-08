/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OMainWnd.cpp
 * Author: onkel
 * 
 * Created on January 1, 2017, 2:37 PM
 */

#include <gtkmm.h>
#include <giomm/simpleactiongroup.h>
#include "OMainWnd.h"
#include <iostream>

OMainWnd::OMainWnd() : Gtk::Window(), m_WorkerThread(nullptr) {
	set_title("Tascam US-16x08 DSP Mixer");

	alsa = new OAlsa();


	Gdk::Color color;
	color.set_rgb_p(0.2, 0.2, 0.2);
	modify_bg(Gtk::STATE_NORMAL, color);
	
	Gtk::MenuBar* menubar = Gtk::manage(new Gtk::MenuBar);
	
	menuitem_file.set_label("File");
	menubar->append(menuitem_file);
	menuitem_file.set_submenu(menu_file);
	m_menubox.add(*menubar);
	menuitem_file_reset.set_label("Reset all");
	menu_file.append(menuitem_file_reset);
	menuitem_file_exit.set_label("Exit");
	menu_file.append(menuitem_file_exit);
	
	menuitem_file_reset.signal_activate().connect(sigc::bind<>(sigc::mem_fun (this, &OMainWnd::on_menu_file_reset), 0));
	menuitem_file_exit.signal_activate().connect(sigc::bind<>(sigc::mem_fun (this, &OMainWnd::on_menu_file_exit), 0));
	
//	m_menubox.set_size_request(-1, 32);
	m_vbox.pack_start(m_menubox, true, false);
	m_vbox.pack_start(m_hbox);
	add(m_vbox);

	if (!alsa->open_device()) {
		for (int i = 0; i < NUM_CHANNELS; i++) {
			m_stripLayouts[i].init(i, alsa);
			m_hbox.pack_start(m_stripLayouts[i]);

		}
		m_master.init(0, alsa);
		m_hbox.pack_start(m_master, false, false);


		show_all_children(true);

		m_Dispatcher.connect(sigc::mem_fun(*this, &OMainWnd::on_notification_from_worker_thread));

		if (m_WorkerThread) {
			std::cout << "Can't start a worker thread while another one is running." << std::endl;
		} else {
			// Start a new worker thread.
			m_WorkerThread = new std::thread(
					[this] {
						m_Worker.do_work(this);
					});
		}

	}
}

OMainWnd::~OMainWnd() {
	m_Worker.stop_work();
	while (!m_Worker.has_stopped())
		sleep(1);
	if (alsa)
		delete alsa;
}

void OMainWnd::notify() {
	m_Dispatcher.emit();
}

void OMainWnd::on_notification_from_worker_thread() {
	if (m_WorkerThread && m_Worker.has_stopped()) {
		if (m_WorkerThread->joinable())
			m_WorkerThread->join();
		delete m_WorkerThread;
		m_WorkerThread = nullptr;
	}
	for (int i = 0; i < NUM_CHANNELS; i++) {
		m_stripLayouts[i].m_meter.setLevel(alsa->sliderTodB(alsa->meters[i] / 32768. * 133.) / 133. * 32768);
		if (m_stripLayouts[i].m_comp.is_active()) {
			m_stripLayouts[i].m_comp.m_reduction.setLevel(alsa->sliderTodB(alsa->meters[i + 18] / 32768. * 133.) / 133. * 32768);
//			  printf("meter: %d, %d\n", (int)(alsa->meters[i + 18]/ 32768. * 133), alsa->sliderTodB(alsa->meters[i + 18] / 133. * 32768.));
		}
		else
			m_stripLayouts[i].m_comp.m_reduction.setLevel(32767);
	}
	m_master.m_meter_left.setLevel(alsa->sliderTodB(alsa->meters[16] / 32768. * 133.) / 133. * 32768);
	m_master.m_meter_right.setLevel(alsa->sliderTodB(alsa->meters[17] / 32768. * 133.) / 133. * 32768);

}

void OMainWnd::on_menu_file_exit(int i) {
	this->hide();
}

void OMainWnd::on_menu_file_reset(int i) {
	m_master.reset(alsa);
	
	for (int i = 0; i < NUM_CHANNELS; i++) {
		m_stripLayouts[i].reset(alsa, i);
	}
}
