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
#include "OMainWnd.h"
#include <iostream>

OMainWnd::OMainWnd() : Gtk::Window(), m_WorkerThread(nullptr) {
	set_title("Tascam US-16x08 DSP Mixer");

	alsa = new OAlsa();

	add(m_hbox);

	if (!alsa->open_device()) {
		for (int i = 0; i < NUM_CHANNELS; i++) {
			m_stripLayouts[i].init(i, alsa);
			m_hbox.pack_start(m_stripLayouts[i]);

		}
		m_master.init(0, alsa);
		m_hbox.pack_start(m_master, false, false);


		show_all_children(true);

		m_Dispatcher.connect(sigc::mem_fun(*this, &OMainWnd::on_notification_from_worker_thread));		
		
		if (m_WorkerThread)
		{
		  std::cout << "Can't start a worker thread while another one is running." << std::endl;
		}
		else
		{
		  // Start a new worker thread.
		  m_WorkerThread = new std::thread(
			[this]
			{
			  m_Worker.do_work(this);
			});
		}

	}
}

OMainWnd::~OMainWnd() {
	m_Worker.stop_work();
	while( !m_Worker.has_stopped() )
		sleep(1);
	if (alsa)
		delete alsa;
}

void OMainWnd::notify()
{
  m_Dispatcher.emit();
}

void OMainWnd::on_notification_from_worker_thread()
{
  if (m_WorkerThread && m_Worker.has_stopped())
  {
    if (m_WorkerThread->joinable())
      m_WorkerThread->join();
    delete m_WorkerThread;
    m_WorkerThread = nullptr;
  }
 	for( int i = 0; i < NUM_CHANNELS; i++) {
		m_stripLayouts[i].m_meter.setLevel(alsa->meters[i]);
	} 
  m_master.m_meter_left.setLevel(alsa->meters[16]);
  m_master.m_meter_right.setLevel(alsa->meters[17]);
}

