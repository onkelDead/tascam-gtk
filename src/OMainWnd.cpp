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
#include <libxml++-2.6/libxml++/libxml++.h>
#include <libxml++-2.6/libxml++/parsers/textreader.h>
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
	menuitem_file_load.set_label("Load values file");
	menu_file.append(menuitem_file_load);
	menuitem_file_save.set_label("Save values file");
	menu_file.append(menuitem_file_save);
	menuitem_file_reset.set_label("Reset all");
	menu_file.append(menuitem_file_reset);
	menuitem_file_exit.set_label("Exit");
	menu_file.append(menuitem_file_exit);

	menuitem_file_load.signal_activate().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_menu_file_load), 0));
	menuitem_file_save.signal_activate().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_menu_file_save), 0));
	menuitem_file_reset.signal_activate().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_menu_file_reset), 0));
	menuitem_file_exit.signal_activate().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_menu_file_exit), 0));

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
		} else
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

void OMainWnd::on_menu_file_save(int i) {

	Gtk::FileChooserDialog dialog("Please choose a file",
			Gtk::FILE_CHOOSER_ACTION_SAVE);

	dialog.set_current_folder("./");
	dialog.set_transient_for(*this);

	Gtk::FileFilter filter_text;
	filter_text.set_name("Tascam values files");
	filter_text.add_mime_type("text/xml");
	dialog.add_filter(filter_text);

	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("Select", Gtk::RESPONSE_OK);

	int result = dialog.run();

	switch (result) {
		case(Gtk::RESPONSE_OK):
			save_values(dialog.get_filename());
			break;
	}

}

void OMainWnd::on_menu_file_load(int i) {

	Gtk::FileChooserDialog dialog("Please choose a file",
			Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog.set_transient_for(*this);

	Gtk::FileFilter filter_text;
	filter_text.set_name("Tascam values files");
	filter_text.add_mime_type("text/xml");
	dialog.add_filter(filter_text);

	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("Select", Gtk::RESPONSE_OK);

	int result = dialog.run();

	switch (result) {
		case(Gtk::RESPONSE_OK):
			load_values(dialog.get_filename());
			break;
	}
	/*
		Glib::ustring = QFileDialog::getSaveFileName(this,
											   tr("Save values to file"), ".",
											   tr("Xml files (*.xml)"));


		if( filename.isEmpty())
			return;
		if( !filename.endsWith(".xml"))
			filename.append(".xml");
		QFile file(filename);
		file.open(QIODevice::WriteOnly);

		QXmlStreamWriter xmlWriter(&file);
		xmlWriter.setAutoFormatting(true);
		xmlWriter.writeStartDocument();

		xmlWriter.writeStartElement("Settings");

		xmlWriter.writeStartElement("Routes");
		for( int i = 0; i < 8; i++) {
		   xmlWriter.writeStartElement("Route");
		   xmlWriter.writeAttribute("index", QString::number(i) );
		   xmlWriter.writeCharacters(QString::number(master->route[i]->currentIndex()) );
		   xmlWriter.writeEndElement();
		}
		xmlWriter.writeEndElement();

		xmlWriter.writeStartElement("Master");
		xmlWriter.writeTextElement("Level", QString::number(master->master_left->value()) );
		xmlWriter.writeEndElement();

		xmlWriter.writeStartElement("Inputs");
		for( int i = 0; i < 16; i++) {
			xmlWriter.writeStartElement("Input");
			xmlWriter.writeAttribute("index", QString::number(i) );
			xmlWriter.writeTextElement("Level", QString::number(ch[i]->fader->value()) );
			xmlWriter.writeTextElement("Mute", QString::number(ch[i]->mute->isChecked()) );
			xmlWriter.writeTextElement("Solo", QString::number(ch[i]->solo->isChecked()) );
			xmlWriter.writeTextElement("Phase", QString::number(ch[i]->phase->isChecked()) );
			xmlWriter.writeTextElement("Pan", QString::number(ch[i]->pan->value()) );
			xmlWriter.writeTextElement("EQ", QString::number(ch[i]->eq_enable->isChecked()) );
			xmlWriter.writeTextElement("EQLowLevel", QString::number(ch[i]->eq_low_level->value()) );
			xmlWriter.writeTextElement("EQLowFrequence", QString::number(ch[i]->eq_low_freq->value()) );
			xmlWriter.writeTextElement("EQMidLowLevel", QString::number(ch[i]->eq_midlow_level->value()) );
			xmlWriter.writeTextElement("EQMidLowFrequence", QString::number(ch[i]->eq_midlow_freq->value()) );
			xmlWriter.writeTextElement("EQMidLowWidth", QString::number(ch[i]->eq_midlow_width->value()) );
			xmlWriter.writeTextElement("EQMidHighLevel", QString::number(ch[i]->eq_midhigh_level->value()) );
			xmlWriter.writeTextElement("EQMidHighFrequence", QString::number(ch[i]->eq_midhigh_freq->value()) );
			xmlWriter.writeTextElement("EQMidHighWidth", QString::number(ch[i]->eq_midhigh_width->value()) );
			xmlWriter.writeTextElement("EQHighLevel", QString::number(ch[i]->eq_high_level->value()) );
			xmlWriter.writeTextElement("EQHighFrequence", QString::number(ch[i]->eq_high_freq->value()) );

			xmlWriter.writeTextElement("Comp", QString::number(ch[i]->cp_enable->isChecked()) );
			xmlWriter.writeTextElement("CompGain", QString::number(ch[i]->cp_gain->value()) );
			xmlWriter.writeTextElement("CompAttack", QString::number(ch[i]->cp_attack->value()) );
			xmlWriter.writeTextElement("CompRelease", QString::number(ch[i]->cp_release->value()) );
			xmlWriter.writeTextElement("CompRatio", QString::number(ch[i]->cp_ratio->value()) );
			xmlWriter.writeTextElement("CompThreshold", QString::number(ch[i]->cp_threshold->value()) );


			xmlWriter.writeEndElement();
		}
		xmlWriter.writeEndElement();
		xmlWriter.writeEndElement();

		file.close();
	 */
}

void OMainWnd::save_values(Glib::ustring filename) {

	if (!filename.find(".xml", 0))
		filename.append(".xml");

	printf("save: %s\n", filename.c_str());

	FILE* file = fopen(filename.c_str(), "w");
	if (file) {
		fprintf(file, "<values>\n");

		fprintf(file, "\t<master>");
		fprintf(file, "%d", (int) m_master.m_fader.get_value());
		fprintf(file, "</master>\n");

		fprintf(file, "\t<mute>");
		fprintf(file, "%d", (int) m_master.m_mute.get_active());
		fprintf(file, "</mute>\n");

		fprintf(file, "\t<bypass>");
		fprintf(file, "%d", (int) m_master.m_true_bypass.get_active());
		fprintf(file, "</bypass>\n");

		fprintf(file, "\t<bus_out>");
		fprintf(file, "%d", (int) m_master.m_comp_to_stereo.get_active());
		fprintf(file, "</bus_out>\n");

		for (int i = 0; i < 8; i++) {

			fprintf(file, "\t<route index=\"%d\">", i);
			fprintf(file, "%d", (int) m_master.m_route[i].get_active_row_number());
			fprintf(file, "</route>\n");

		}

		for (int j = 0; j < NUM_CHANNELS; j++) {
			fprintf(file, "\t<channel index=\"%d\">\n", j);
			m_stripLayouts[j].save_values(file);
			fprintf(file, "\t</channel>\n");
		}

		fprintf(file, "</values>\n");
		fclose(file);
	}

}

void OMainWnd::load_values(Glib::ustring filename) {

	try {
		xmlpp::TextReader reader(filename);

		while (reader.read()) {
			if( !strcmp(reader.get_name().c_str(), "master") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_master.m_fader.set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if( !strcmp(reader.get_name().c_str(), "mute") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_master.m_mute.set_active(atoi(reader.get_value().c_str()) == 1);
				m_master.m_mute.toggled();
				usleep(RESET_VALUE_DELAY);
			}			
			if( !strcmp(reader.get_name().c_str(), "bypass") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_master.m_true_bypass.set_active(atoi(reader.get_value().c_str()) == 1);
				m_master.m_true_bypass.toggled();
				usleep(RESET_VALUE_DELAY);
			}			
			if( !strcmp(reader.get_name().c_str(), "bus_out") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_master.m_comp_to_stereo.set_active(atoi(reader.get_value().c_str()) == 1);
				m_master.m_comp_to_stereo.toggled();
				usleep(RESET_VALUE_DELAY);
			}			
			
			if( !strcmp(reader.get_name().c_str(), "route") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				if(reader.has_attributes()) {
					reader.move_to_first_attribute();
					int index = atoi(reader.get_value().c_str());
					reader.read();
					m_master.m_route[index].set_active(atoi(reader.get_value().c_str()));
					usleep(RESET_VALUE_DELAY);
				}
			}
			
			if( !strcmp(reader.get_name().c_str(), "channel") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				if(reader.has_attributes()) {
					reader.move_to_first_attribute();
					int index = atoi(reader.get_value().c_str());
					m_stripLayouts[index].load_values(reader.read_outer_xml());
				}
			}
			
		}
	} catch (const std::exception& e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return;
	}

}
