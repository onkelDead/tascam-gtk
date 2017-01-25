/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OFader.cpp
 * Author: onkel
 * 
 * Created on January 15, 2017, 10:17 AM
 */

#include <gtkmm.h>
#include <stdbool.h>
#include <libxml++-2.6/libxml++/libxml++.h>
#include <libxml++-2.6/libxml++/parsers/textreader.h>
#include "OMainWnd.h"

#include "OFader.h"

OFader::OFader() : Gtk::VBox() {
	m_dB.set_name("db-label");
	for (int mi = 0; mi < 2; mi++) {
		m_meter[mi].set_size_request(10, 160);
		m_meter[mi].set_level_color(0, .7, 0, 1);
		m_meter[mi].set_hexpand(false);
		m_meter[mi].set_halign(Gtk::ALIGN_CENTER);
	}
	m_view_type = HIDDEN;
	m_grid.set_halign(Gtk::ALIGN_CENTER);
	m_grid.set_hexpand(true);	
	add(m_grid);
}

OFader::OFader(const OFader& orig) {
}

OFader::~OFader() {
}

void OFader::set_view_type(VIEW_TYPE view_type, CHANNEL_TYPE channel_type) {

	if( view_type == HIDDEN) {
		std::vector<Gtk::Widget*> childList = m_grid.get_children();
		std::vector<Gtk::Widget*>::iterator it;

		for (it = childList.begin(); it < childList.end(); it++) {
			m_grid.remove(**it);
		}
	}

	if (view_type == NORMAL) {
		if (channel_type == MONO) {
			m_grid.attach(*m_MuteEnable, 0, 0, 1, 1);
			m_grid.attach(*m_SoloEnable, 0, 1, 1, 1);
			m_grid.attach(*m_Pan[0], 1, 0, 1, 2);
			m_grid.attach(*m_PhaseEnable[0], 1, 2, 1, 1);
			m_grid.attach(m_dB, 0, 2, 1, 1);
			m_grid.attach(*m_fader, 0, 3, 1, 1);
			m_grid.attach(m_meter[0], 1, 3, 1, 1);

		}
		if (channel_type == STEREO) {

			m_grid.attach(*m_MuteEnable, 0, 0, 1, 1);
			m_grid.attach(*m_SoloEnable, 0, 1, 1, 1);
			m_grid.attach(*m_Pan[0], 1, 0, 1, 2);
			m_grid.attach(*m_Pan[1], 2, 0, 1, 2);
			m_grid.attach(*m_PhaseEnable[0], 1, 2, 1, 1);
			m_grid.attach(*m_PhaseEnable[1], 2, 2, 1, 1);
			m_grid.attach(m_dB, 0, 2, 1, 1);
			m_grid.attach(*m_fader, 0, 3, 1, 1);
			m_grid.attach(m_meter[0], 1, 3, 1, 1);
			m_grid.attach(m_meter[1], 2, 3, 1, 1);
		}
	}
	if (view_type == COMPACT) {
		if (channel_type == MONO) {
			m_grid.attach(*m_MuteEnable, 0, 0, 3, 1);
			m_grid.attach(*m_SoloEnable, 0, 1, 3, 1);
			m_grid.attach(*m_Pan[0], 0, 2, 3, 1);
			m_grid.attach(*m_PhaseEnable[0], 0, 3, 3, 1);
			m_grid.attach(m_dB, 0, 4, 3, 1);
			m_grid.attach(*m_fader, 0, 5, 1, 1);
			m_grid.attach(m_meter[0], 1, 5, 1, 1);
		}
		if (channel_type == STEREO) {

			m_grid.attach(*m_MuteEnable, 0, 0, 4, 1);
			m_grid.attach(*m_SoloEnable, 0, 1, 4, 1);
			m_grid.attach(*m_Pan[0], 0, 2, 2, 1);
			m_grid.attach(*m_Pan[1], 2, 2, 2, 1);
			m_grid.attach(*m_PhaseEnable[0], 0, 3, 2, 1);
			m_grid.attach(*m_PhaseEnable[1], 2, 3, 2, 1);
			m_grid.attach(m_dB, 0, 4, 4, 1);
			m_grid.attach(*m_fader, 0, 5, 2, 1);
			m_grid.attach(m_meter[0], 2, 5, 1, 1);
			m_grid.attach(m_meter[1], 3, 5, 1, 1);
		}
	}
	show_all_children(true);
	
	m_view_type = view_type;
}

void OFader::init(int index, OAlsa* alsa, Gtk::Window * wnd) {

	char l_title[64];
	int val;

	OMainWnd* wnd_ = (OMainWnd*) wnd;

	m_fader = &wnd_->m_fader[index];
	val = alsa->getInteger(CTL_NAME_FADER, index);
	m_fader->set_value(alsa->dBToSlider(val) + 1);

	snprintf(l_title, sizeof (l_title), "%d dB", val - 127);
	m_fader->set_tooltip_text(l_title);
	m_dB.set_label(l_title);

	m_fader->signal_value_changed().connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_fader_changed), index, CTL_NAME_FADER, m_fader, &m_dB));

	m_Pan[0] = &wnd_->m_Pan[index];
	m_Pan[0]->set_value(alsa->getInteger(CTL_NAME_PAN, index));
	m_Pan[0]->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index, CTL_NAME_PAN));
	m_Pan[0]->set_hexpand(false);
	m_Pan[0]->set_halign(Gtk::ALIGN_CENTER);

	m_MuteEnable = &wnd_->m_MuteEnable[index];
	m_MuteEnable->set_active(alsa->getBoolean(CTL_NAME_MUTE, index));
	m_MuteEnable->signal_toggled().connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_tb_changed), index, CTL_NAME_MUTE));

	m_SoloEnable = &wnd_->m_SoloEnable[index];
	m_SoloEnable->set_active(alsa->getBoolean(CTL_NAME_SOLO, index));
	m_SoloEnable->signal_toggled().connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_tb_changed), index, CTL_NAME_SOLO));

	m_PhaseEnable[0] = &wnd_->m_PhaseEnable[index];
	m_PhaseEnable[0]->set_active(alsa->getBoolean(CTL_NAME_PHASE, index));
	m_PhaseEnable[0]->signal_toggled().connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_tb_changed), index, CTL_NAME_PHASE));


	if (!(index % 2)) {
		m_Pan[1] = &wnd_->m_Pan[index + 1];
		m_Pan[1]->set_value(alsa->getInteger(CTL_NAME_PAN, index + 1));
		m_Pan[1]->signal_value_changed.connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_dial_changed), index + 1, CTL_NAME_PAN));
		m_Pan[1]->set_hexpand(false);
		m_Pan[1]->set_halign(Gtk::ALIGN_CENTER);
		m_PhaseEnable[1] = &wnd_->m_PhaseEnable[index + 1];
		m_PhaseEnable[1]->set_active(alsa->getBoolean(CTL_NAME_PHASE, index + 1));
		m_PhaseEnable[1]->signal_toggled().connect(sigc::bind<>(sigc::mem_fun(wnd_, &OMainWnd::on_ch_tb_changed), index + 1, CTL_NAME_PHASE));
	}
}

void OFader::reset(OAlsa* alsa, int index) {

	m_fader->set_value(alsa->dBToSlider(127) + 1);
	usleep(RESET_VALUE_DELAY);

	m_Pan[0]->reset();
	usleep(RESET_VALUE_DELAY);


	alsa->setBoolean(CTL_NAME_MUTE, 0, 0);
	m_MuteEnable->set_active(alsa->getBoolean(CTL_NAME_MUTE, index));
	usleep(RESET_VALUE_DELAY);

	alsa->setBoolean(CTL_NAME_SOLO, 0, 0);
	m_SoloEnable->set_active(alsa->getBoolean(CTL_NAME_SOLO, index));
	usleep(RESET_VALUE_DELAY);

	alsa->setBoolean(CTL_NAME_PHASE, 0, 0);
	m_PhaseEnable[0]->set_active(alsa->getBoolean(CTL_NAME_PHASE, index));
	usleep(RESET_VALUE_DELAY);

}

void OFader::save_values(FILE * file) {

	fprintf(file, "\t\t<fader>");
	fprintf(file, "%d", (int) m_fader->get_value());
	fprintf(file, "</fader>\n");

	fprintf(file, "\t\t<pan>");
	fprintf(file, "%d", (int) m_Pan[0]->get_value());
	fprintf(file, "</pan>\n");

	fprintf(file, "\t\t<mute>");
	fprintf(file, "%d", (int) m_MuteEnable->get_active());
	fprintf(file, "</mute>\n");

//	fprintf(file, "\t\t<solo>");
//	fprintf(file, "%d", (int) m_SoloEnable->get_active());
//	fprintf(file, "</solo>\n");

	fprintf(file, "\t\t<phase>");
	fprintf(file, "%d", (int) m_PhaseEnable[0]->get_active());
	fprintf(file, "</phase>\n");

}

void OFader::load_values(Glib::ustring xml) {

	try {
		xmlpp::TextReader reader((const unsigned char*) xml.c_str(), xml.size());

		while (reader.read()) {
			if (!strcmp(reader.get_name().c_str(), "fader") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_fader->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "pan") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_Pan[0]->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "pan") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_Pan[0]->set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "mute") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_MuteEnable->set_active(atoi(reader.get_value().c_str()) == 1);
				m_MuteEnable->toggled();
				usleep(RESET_VALUE_DELAY);
			}

			if (!strcmp(reader.get_name().c_str(), "phase") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_PhaseEnable[0]->set_active(atoi(reader.get_value().c_str()) == 1);
				m_PhaseEnable[0]->toggled();
				usleep(RESET_VALUE_DELAY);
			}
		}

	} catch (const std::exception& e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return;
	}
}