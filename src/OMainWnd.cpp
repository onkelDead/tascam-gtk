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
	//	modify_bg(Gtk::STATE_NORMAL, color);

	Gtk::MenuBar* menubar = Gtk::manage(new Gtk::MenuBar);

	m_refActionGroup = Gtk::ActionGroup::create();
	m_refActionGroup->add(Gtk::Action::create("File", "_File"));
	m_refActionGroup->add(Gtk::Action::create("load", Gtk::Stock::OPEN, "_Load values..."),
			sigc::mem_fun(this, &OMainWnd::on_menu_file_load));
	m_refActionGroup->add(Gtk::Action::create("save", Gtk::Stock::SAVE, "_Save values..."),
			sigc::mem_fun(this, &OMainWnd::on_menu_file_save));
	m_refActionGroup->add(Gtk::Action::create("reset", Gtk::Stock::REVERT_TO_SAVED, "_Reset all"),
			sigc::mem_fun(this, &OMainWnd::on_menu_file_reset));
	m_refActionGroup->add(Gtk::Action::create("exit", Gtk::Stock::QUIT),
			sigc::mem_fun(this, &OMainWnd::on_menu_file_exit));

	m_refUIManager = Gtk::UIManager::create();
	m_refUIManager->insert_action_group(m_refActionGroup);

	//Layout the actions in a menubar and toolbar:
	Glib::ustring ui_info =
			"<ui>"
			"  <menubar name='MenuBar'>"
			"    <menu action='File'>"
			"        <menuitem action='load'/>"
			"        <menuitem action='save'/>"
			"        <menuitem action='reset'/>"
			"        <separator />"
			"        <menuitem action='exit' />"
			"    </menu>"
			"  </menubar>"
			"</ui>";

	try {
		m_refUIManager->add_ui_from_string(ui_info);
	} catch (const Glib::Error& ex) {
		std::cerr << "building menus failed: " << ex.what();
	}

	Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");
	if (!(pMenubar)) {
		g_warning("GMenu or AppMenu not found");
	} else {
		m_menubox.pack_start(*pMenubar, false, false);
	}

	menu_popup_load.set_label("Load channel values");
	menu_popup.append(menu_popup_load);
	menu_popup_save.set_label("Save channel values");
	menu_popup.append(menu_popup_save);
	menu_popup_reset.set_label("Reset channel values");
	menu_popup.append(menu_popup_reset);

	m_vbox.pack_start(m_menubox, false, false);
	m_menubox.set_name("menu");
	
	
	m_vbox.pack_start(m_hbox);
	add(m_vbox);

	if (!alsa->open_device()) {
		for (int i = 0; i < NUM_CHANNELS; i++) {
			m_stripLayouts[i].init(i, alsa);
			m_stripLayouts[i].m_event_box.signal_button_press_event().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_title_context), i));
			m_hbox.pack_start(m_stripLayouts[i], false, false);
			m_stripLayouts[i].set_size_request(80, -1);
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
	set_name("OMainWnd");

	m_refCssProvider = Gtk::CssProvider::create();
	auto refStyleContext = get_style_context();
	refStyleContext->add_provider(m_refCssProvider,
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	try {
		m_refCssProvider->load_from_path("src/tascam-gtk.css");
	} catch (const Gtk::CssProviderError& ex) {
		std::cerr << "CssProviderError, Gtk::CssProvider::load_from_path() failed: "
				<< ex.what() << std::endl;
	} catch (const Glib::Error& ex) {
		std::cerr << "Error, Gtk::CssProvider::load_from_path() failed: "
				<< ex.what() << std::endl;
	}

//	auto refStyleContext = get_style_context();
	auto screen = Gdk::Screen::get_default();
	refStyleContext->add_provider_for_screen(
			Gdk::Screen::get_default(),
			m_refCssProvider,
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
			);

}

OMainWnd::~OMainWnd() {
	m_Worker.stop_work();
	while (!m_Worker.has_stopped())
		sleep(1);
	if (alsa)
		delete alsa;
}

bool OMainWnd::on_title_context(GdkEventButton* event, int channel_index) {
	if (event->button == 3) {

		if (!m_popup_load_connection.empty())
			m_popup_load_connection.disconnect();
		if (!m_popup_save_connection.empty())
			m_popup_save_connection.disconnect();
		if (!m_popup_reset_connection.empty())
			m_popup_reset_connection.disconnect();

		m_popup_load_connection = menu_popup_load.signal_activate().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_menu_popup_load), channel_index));
		m_popup_save_connection = menu_popup_save.signal_activate().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_menu_popup_save), channel_index));
		m_popup_reset_connection = menu_popup_reset.signal_activate().connect(sigc::bind<>(sigc::mem_fun(this, &OMainWnd::on_menu_popup_reset), channel_index));


		menu_popup.show_all();
		menu_popup.popup(3, event->time);
	}
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

void OMainWnd::on_menu_file_exit() {
	this->hide();
}

void OMainWnd::on_menu_file_reset() {
	m_master.reset(alsa);

	for (int i = 0; i < NUM_CHANNELS; i++) {
		m_stripLayouts[i].reset(alsa, i);
	}
}

void OMainWnd::on_menu_file_save() {

	Gtk::FileChooserDialog dialog("Please choose a file",
			Gtk::FILE_CHOOSER_ACTION_SAVE);

	dialog.set_current_folder("./");
	dialog.set_transient_for(*this);

	auto filter_text = Gtk::FileFilter::create();
	filter_text->set_name("Tascam values files");
	filter_text->add_mime_type("text/xml");
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

void OMainWnd::on_menu_file_load() {

	Gtk::FileChooserDialog dialog("Please choose a file",
			Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog.set_current_folder("./");
	dialog.set_transient_for(*this);

	auto filter_text = Gtk::FileFilter::create();
	filter_text->set_name("Tascam values files");
	filter_text->add_mime_type("text/xml");
	dialog.add_filter(filter_text);

	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("Select", Gtk::RESPONSE_OK);

	int result = dialog.run();

	dialog.hide();
	switch (result) {
		case(Gtk::RESPONSE_OK):
			load_values(dialog.get_filename());
			break;
	}
}

void OMainWnd::on_menu_popup_load(int channel_index) {
	char l_title[256];

	snprintf(l_title, sizeof (l_title), "Select Mixer file to load in %s", m_stripLayouts[channel_index].m_title.get_label().c_str());

	Gtk::FileChooserDialog dialog(l_title, Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog.set_current_folder("./");
	dialog.set_transient_for(*this);

	auto filter_text = Gtk::FileFilter::create();
	filter_text->set_name("Tascam values files");
	filter_text->add_mime_type("text/xml");
	dialog.add_filter(filter_text);

	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("Select", Gtk::RESPONSE_OK);

	int result = dialog.run();

	dialog.hide();
	switch (result) {
		case(Gtk::RESPONSE_OK):
			load_channel_values(dialog.get_filename(), channel_index);
			break;
	}

}

void OMainWnd::on_menu_popup_save(int channel_index) {
	printf("save %d\n", channel_index);

	Gtk::FileChooserDialog dialog("Please choose a file",
			Gtk::FILE_CHOOSER_ACTION_SAVE);

	dialog.set_current_folder("./");
	dialog.set_transient_for(*this);

	auto filter_text = Gtk::FileFilter::create();
	filter_text->set_name("Tascam values files");
	filter_text->add_mime_type("text/xml");
	dialog.add_filter(filter_text);

	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("Select", Gtk::RESPONSE_OK);

	int result = dialog.run();

	switch (result) {
		case(Gtk::RESPONSE_OK):
			save_channel_values(dialog.get_filename(), channel_index);
			break;
	}

}

void OMainWnd::on_menu_popup_reset(int i) {
	m_stripLayouts[i].reset(alsa, i);
}

void OMainWnd::save_channel_values(Glib::ustring filename, int channel_index) {
	if (!strstr(filename.c_str(), ".xml"))
		filename.append(".xml");

	FILE* file = fopen(filename.c_str(), "w");
	if (file) {
		fprintf(file, "<channel>\n");
		m_stripLayouts[channel_index].save_values(file, 1);
		fprintf(file, "</channel>\n");
		fclose(file);
	}
}

void OMainWnd::load_channel_values(Glib::ustring filename, int channel_index) {

	try {
		xmlpp::TextReader reader(filename);

		while (reader.read()) {
			if (!strcmp(reader.get_name().c_str(), "channel") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				m_stripLayouts[channel_index].load_values(reader.read_outer_xml());
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return;
	}

}

void OMainWnd::save_values(Glib::ustring filename) {

	if (!strstr(filename.c_str(), ".xml"))
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
			if (!strcmp(reader.get_name().c_str(), "master") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_master.m_fader.set_value(atoi(reader.get_value().c_str()));
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "mute") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_master.m_mute.set_active(atoi(reader.get_value().c_str()) == 1);
				m_master.m_mute.toggled();
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "bypass") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_master.m_true_bypass.set_active(atoi(reader.get_value().c_str()) == 1);
				m_master.m_true_bypass.toggled();
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "bus_out") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				reader.read();
				m_master.m_comp_to_stereo.set_active(atoi(reader.get_value().c_str()) == 1);
				m_master.m_comp_to_stereo.toggled();
				usleep(RESET_VALUE_DELAY);
			}
			if (!strcmp(reader.get_name().c_str(), "route") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				if (reader.has_attributes()) {
					reader.move_to_first_attribute();
					int index = atoi(reader.get_value().c_str());
					reader.read();
					m_master.m_route[index].set_active(atoi(reader.get_value().c_str()));
					usleep(RESET_VALUE_DELAY);
				}
			}
			if (!strcmp(reader.get_name().c_str(), "channel") && reader.get_node_type() != xmlpp::TextReader::xmlNodeType::EndElement) {
				if (reader.has_attributes()) {
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
