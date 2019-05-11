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

#include <gtkmm-3.0/gtkmm.h>
//#include <gtkmm-2.4/gtkmm/scale.h>
//#include <gtkmm-2.4/gtkmm/togglebutton.h>
//#include <gtkmm-2.4/gtkmm/comboboxtext.h>
#include "ODial.h"
#include "OAlsa.h"

//* EQ control element names.
const char* eq_control_path[] = {
	"name='9 EQ',index=",
	"name='81 HighFreq',index=",
	"name='8 High',index=",
	"name='71 MHiFreq',index=",
	"name='72 MHiWidth',index=",
	"name='7 MHigh',index=",
	"name='61 MLowFreq',index=",
	"name='62 MLowWidth',index=",
	"name='6 MLow',index=",
	"name='51 LowFreq',index=",
	"name='5 Low',index=",
	NULL
};

//* Compressor control element names.
const char* comp_control_path[] = {
	"name='A Comp',index=",
	"name='B Thresh',index=",
	"name='C Ratio',index=",
	"name='D Attack',index=",
	"name='E Release',index=",
	"name='F Gain',index=",
	NULL
};

OAlsa::OAlsa() {
	cardnum = -1;
}

OAlsa::~OAlsa() {
}

//#define DEBUG

int OAlsa::get_alsa_cardnum() {
	int card = -1;
	int err = 0;
	char name[32];
	snd_ctl_t *handle;
	snd_ctl_card_info_t *cinfo;

	if (cardnum != -1)
		return cardnum;

	snd_ctl_card_info_alloca(&cinfo);

	cardnum = -1;

	if (snd_card_next(&card) < 0 || card < 0) {
		fprintf(stderr, "ERROR: No sound card found.\n");
		return -1;
	}
	while (card >= 0) {
		sprintf(name, "hw:%d", card);

		if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
			fprintf(stderr, "ERROR: Control %s open error: %s\n", name, snd_strerror(err));
			goto next_card;
		}
#ifdef DEBUG
		fprintf(stdout, "card opened\n");
#endif
		if ((err = snd_ctl_card_info(handle, cinfo)) < 0) {
			fprintf(stderr, "ERROR: Control hardware info (%i): %s", card, snd_strerror(err));
			snd_ctl_close(handle);
			goto next_card;
		}
#ifdef DEBUG
		fprintf(stdout, "device: [%s] %s\n", snd_ctl_card_info_get_id(cinfo), snd_ctl_card_info_get_name(cinfo));
#endif
		if (strcmp("US16x08", snd_ctl_card_info_get_id(cinfo)) == 0) {
			cardnum = card;
			return cardnum;
		} else {
			snd_ctl_close(handle);
#ifdef DEBUG
			fprintf(stderr, "card closed\n");
#endif
		}
next_card:
		if (snd_card_next(&card) < 0) {
			fprintf(stderr, "ERROR: snd_card_next failed.\n");
			break;
		}
	}
	if (cardnum == -1) {
		fprintf(stderr, "ERROR: No proper sound card found.\n");
		return -1;
	}
	return cardnum;
}

int OAlsa::open_device() {
	int err = 0;
	int i = 0, j;
	char name[32];

	if (cardnum == -1) {
		cardnum = get_alsa_cardnum();
		if (cardnum == -1)
			return -1;
	}

	sprintf(name, "hw:%d", cardnum);
	if ((err = snd_hctl_open(&hctl, name, 0)) < 0) {
		fprintf(stderr, "Control %s open error: %s\n", name, snd_strerror(err));
		return -1;
	}
	if ((err = snd_hctl_load(hctl)) < 0) {
		fprintf(stderr, "Control %s load error: %s\n", name, snd_strerror(err));
		return -1;
	}



	return 0;
}

void OAlsa::close_device() {
    int i = 0, j;

	if (hctl) {
		snd_hctl_close(hctl);
		hctl = 0;
	}
}

snd_hctl_elem_t* OAlsa::get_ctrl_by_elem(const char* name) {
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;

    int err = snd_ctl_ascii_elem_id_parse(id, name);
    elem = snd_hctl_find_elem(hctl, id);
    return elem;
}
        
char* OAlsa::create_ctrl_elem_name(const char* name, int index, char* result[], size_t size) {

    snprintf(*result, size, "%s,index=%d", name, index);
    return *result;
}

int OAlsa::getInteger(const char* name, int channel_index) {
    int val = 0;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;
	
	char elem_name[strlen(name) + strlen(CTL_NAME_INDEX_SUFFIX) + 6];
	
	sprintf(elem_name, CTL_NAME_INDEX_SUFFIX, name, channel_index);
	
    int err = snd_ctl_ascii_elem_id_parse(id, elem_name);
    if (err) {
        fprintf(stderr, "Wrong control identifier: %s (%d)\n", name, err);
        return -1;
    }
    elem = snd_hctl_find_elem(hctl, id);
    if (elem) {
        snd_ctl_elem_value_t *control;
        snd_ctl_elem_value_alloca(&control);
        if ((err = snd_hctl_elem_read(elem, control)) < 0) {
            fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
            return -5;
        }
        val = snd_ctl_elem_value_get_integer(control, 0);

    }
    return val;

}

void OAlsa::setInteger(const char* name, int channel_index, int value) {
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;

	char elem_name[strlen(name) + strlen(CTL_NAME_INDEX_SUFFIX) + 6];
	sprintf(elem_name, CTL_NAME_INDEX_SUFFIX, name, channel_index);
	
    int err = snd_ctl_ascii_elem_id_parse(id, elem_name);
    elem = snd_hctl_find_elem(hctl, id);
    if (elem) {
        fflush(stdout);
        snd_ctl_elem_value_t *control;
        snd_ctl_elem_value_alloca(&control);
        snd_ctl_elem_value_set_integer(control, 0, value);
        if ((err = snd_hctl_elem_write(elem, control)) < 0) {
            fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
            return;
        }
		 
		
    }
	else 
		fprintf(stderr, "Control %s element not found\n", elem_name);
}

bool OAlsa::getBoolean(const char* name, int channel_index) {
    int val = 0;
    snd_ctl_elem_id_t *id;	
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;
	
	char elem_name[strlen(name) + strlen(CTL_NAME_INDEX_SUFFIX) + 6];
	sprintf(elem_name, CTL_NAME_INDEX_SUFFIX, name, channel_index);
	
    int err = snd_ctl_ascii_elem_id_parse(id, elem_name);
    if (err) {
        fprintf(stderr, "Wrong control identifier: %s (%d)\n", name, err);
        return -1;
    }
    elem = snd_hctl_find_elem(hctl, id);
    if (elem) {
        snd_ctl_elem_value_t *control;
        snd_ctl_elem_value_alloca(&control);
        if ((err = snd_hctl_elem_read(elem, control)) < 0) {
            fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
            return -5;
        }
        val = snd_ctl_elem_value_get_boolean(control, 0);

    }
    return val;

}

void OAlsa::setBoolean( const char* name, int channel_index, bool value) {
    snd_ctl_elem_id_t *id;	
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;
	
	char elem_name[strlen(name) + strlen(CTL_NAME_INDEX_SUFFIX) + 6];
	sprintf(elem_name, CTL_NAME_INDEX_SUFFIX, name, channel_index);

    int err = snd_ctl_ascii_elem_id_parse(id, elem_name);
    elem = snd_hctl_find_elem(hctl, id);
    if (elem) {
        snd_ctl_elem_value_t *control;
        snd_ctl_elem_value_alloca(&control);
        snd_ctl_elem_value_set_boolean(control, 0, value);
        if ((err = snd_hctl_elem_write(elem, control)) < 0) {
            fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
            return;
        }
    }
}

int OAlsa::getIntegers(const char* name, int vals[], int count) {
    int val = 0;
    snd_ctl_elem_id_t *id;	
    snd_ctl_elem_id_alloca(&id);	
	snd_hctl_elem_t *elem;

    int err = snd_ctl_ascii_elem_id_parse(id, name);
    if (err) {
        fprintf(stderr, "Wrong control identifier: %s (%d)\n", name, err);
        return -1;
    }
    elem = snd_hctl_find_elem(hctl, id);
    if (elem) {
        snd_ctl_elem_value_t *control;
        snd_ctl_elem_value_alloca(&control);
        if ((err = snd_hctl_elem_read(elem, control)) < 0) {
            fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
            return -5;
        }
        for( val = 0; val < count; val++)
            vals[val] = snd_ctl_elem_value_get_integer(control, val);

    }
    return val;
}


void OAlsa::on_combo_control_changed (int n, const char* control_name, Gtk::ComboBoxText* control) {
	int val = control->get_active_row_number();
	setInteger(control_name, n, val);
}

void OAlsa::on_dial_control_changed (int n, const char* control_name, ODial* control) {
	setInteger(control_name, n, control->get_value());
}
void OAlsa::on_toggle_button_control_changed (int n, const char* control_name, Gtk::ToggleButton* control) {
	setBoolean(control_name, n, control->get_active());
}

void OAlsa::on_active_button_control_changed (int n, const char* control_name, Gtk::ToggleButton* control) {
    if( control->get_active() )
            setInteger(control_name, 0, n+1);
    else
            setInteger(control_name, 0, 0);        
}


void OAlsa::on_range_control_changed (int n, const char* control_name, Gtk::VScale* control, Gtk::Label* label) {
	char l_title[64];
	int val = control->get_value();
	int dB = sliderTodB(control->get_value());
	
	setInteger(control_name, n, dB);
	
	snprintf(l_title, sizeof(l_title), "%d dB", dB - 127);
	control->set_tooltip_text(l_title);	
	if( label )
		label->set_label(l_title);
}


int OAlsa::sliderTodB(int pos) {
	return 146.2 - 146.3 / ( pow(10, pos / 127.) );
}

int OAlsa::dBToSlider(int dB) {
	return 127 * log10( 146.3 / (146.2 - dB));
}
