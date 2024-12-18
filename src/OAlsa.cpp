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
#include <gtkmm-3.0/gtkmm/widget.h>
#include "OMainWnd.h"
#include "ODial.h"
#include "OAlsa.h"

snd_hctl_t *g_hctl;

OMainWnd* g_main_wnd;

static void events_value(snd_hctl_elem_t *helem) {
    int val = 0;
    int err = 0;

    snd_ctl_elem_value_t *control;

    snd_ctl_elem_value_alloca(&control);

    if ((err = snd_hctl_elem_read(helem, control)) < 0) {
        fprintf(stderr, "Control %s event element read error: %s\n", "hw:0", snd_strerror(err));
        return;
    }
    val = snd_ctl_elem_value_get_integer(control, 0);
    g_main_wnd->alsa_update_control(helem, val, snd_ctl_elem_value_get_index(control));
}

static void events_info(snd_hctl_elem_t *helem) {
#if 0
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_get_id(helem, id);
    printf("event info: ");
    show_control_id(id);
    printf("\n");
#endif
}

static int element_callback(snd_hctl_elem_t *elem, unsigned int mask) {
    if (mask == SND_CTL_EVENT_MASK_REMOVE) {
        return 0;
    }
    if (mask & SND_CTL_EVENT_MASK_INFO)
        events_info(elem);
    if (mask & SND_CTL_EVENT_MASK_VALUE)
        events_value(elem);
    return 0;
}

static void events_add(snd_hctl_elem_t *helem) {
    snd_hctl_elem_set_callback(helem, element_callback);
    g_main_wnd->alsa_add_control(helem);
}

static int ctl_callback(snd_hctl_t *ctl, unsigned int mask,
        snd_hctl_elem_t *elem) {
    if (mask & SND_CTL_EVENT_MASK_ADD)
        events_add(elem);
    return 0;
}

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

OAlsa::OAlsa(OMainWnd* main_wnd) :
m_Mutex(),
m_shall_stop(false),
m_has_stopped(false) {
    cardnum = -1;
    g_main_wnd = main_wnd;
}

OAlsa::~OAlsa() {
}

//#define DEBUG

int OAlsa::get_alsa_cardnum() {
    int card = -1;
    int err = 0;
    char name[32];
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
        } else {
#ifdef DEBUG
            fprintf(stdout, "card opened\n");
#endif
            if ((err = snd_ctl_card_info(handle, cinfo)) < 0) {
                fprintf(stderr, "ERROR: Control hardware info (%i): %s", card, snd_strerror(err));
                snd_ctl_close(handle);
            } else {
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
            }
            if (snd_card_next(&card) < 0) {
                fprintf(stderr, "ERROR: snd_card_next failed.\n");
                break;
            }
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

    g_hctl = hctl;
    snd_hctl_set_callback(hctl, ctl_callback);

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

    if (!hctl)
        return 0;

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
            fprintf(stderr, "Control %s element %s read error: %s\n", "hw:0", elem_name, snd_strerror(err));
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
            fprintf(stderr, "Control %s element %s read error: %s\n", "hw:0", elem_name, snd_strerror(err));
            return;
        }


    } else
        fprintf(stderr, "Control %s element not found\n", elem_name);
}

bool OAlsa::getBoolean(const char* name, int channel_index) {
    int val = 0;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;

    if (!hctl)
        return false;

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
            fprintf(stderr, "Control %s element %s read error: %s\n", "hw:0", elem_name, snd_strerror(err));
            return -5;
        }
        val = snd_ctl_elem_value_get_boolean(control, 0);

    }
    return val;

}

void OAlsa::setBoolean(const char* name, int channel_index, bool value) {
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
            fprintf(stderr, "Control %s element %s read error: %s\n", "hw:0", elem_name, snd_strerror(err));
            return;
        }
    }
}

int OAlsa::getControlIntegers(snd_hctl_elem_t *elem, int vals[], int count) {
    int val = 0;
    int err;

    snd_ctl_elem_value_t *control;
    snd_ctl_elem_value_alloca(&control);

    if ((err = snd_hctl_elem_read(elem, control)) < 0) {
        fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
        return -5;
    }
    for (val = 0; val < count; val++)
        vals[val] = snd_ctl_elem_value_get_integer(control, val);
    return val;
}

snd_hctl_elem_t* OAlsa::getElement(const char* name) {
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    int err = snd_ctl_ascii_elem_id_parse(id, name);
    if (err) {
        fprintf(stderr, "Wrong control identifier: %s (%d)\n", name, err);
        return NULL;
    }
    return snd_hctl_find_elem(hctl, id);
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
            fprintf(stderr, "Control %s element %s read error: %s\n", "hw:0", name, snd_strerror(err));
            return -5;
        }
        for (val = 0; val < count; val++)
            vals[val] = snd_ctl_elem_value_get_integer(control, val);

    }
    return val;
}

void OAlsa::on_combo_control_changed(int n, const char* control_name, Gtk::ComboBoxText* control) {
    int val = control->get_active_row_number();
    setInteger(control_name, n, val);
}

void OAlsa::on_dial_control_changed(int n, const char* control_name, ODial* control) {
    setInteger(control_name, n, control->get_value());
}

void OAlsa::on_toggle_button_control_changed(int n, const char* control_name, Gtk::ToggleButton* control) {
    setBoolean(control_name, n, control->get_active());
}

void OAlsa::on_active_button_control_changed(int n, const char* control_name, Gtk::ToggleButton* control) {
    if (control->get_active())
        setInteger(control_name, 0, n + 1);
    else
        setInteger(control_name, 0, 0);
}

void OAlsa::on_range_control_changed(int n, const char* control_name, Gtk::VScale* control, Gtk::Label* label) {
    char l_title[64];
    int val = control->get_value();
    int dB = sliderTodB(control->get_value());

    setInteger(control_name, n, dB);

    snprintf(l_title, sizeof (l_title), "%d dB", dB - 127);
    control->set_tooltip_text(l_title);
    if (label)
        label->set_label(l_title);
}

int OAlsa::sliderTodB(int pos) {
    return 146.2 - 146.3 / (pow(10, pos / 127.));
}

int OAlsa::dBToSlider(int dB) {
    return 127 * log10(146.3 / (146.2 - dB));
}

void OAlsa::stop_work() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_shall_stop = true;
}

bool OAlsa::has_stopped() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_has_stopped;
}

void OAlsa::do_work(OMainWnd* caller) {
    m_has_stopped = false;
    m_caller = caller;

    struct pollfd *pollfds = NULL;
    int nfds = 0, n;
    GtkWidget *active_widget;
    unsigned short revents;
    int key;
    int err;
    int res;

    while (!m_shall_stop) {
        res = snd_hctl_wait(hctl, 1000);
        if (res >= 0) {
            res = snd_hctl_handle_events(hctl);
            m_shall_stop = true;
        }
    }
    free(pollfds);

    m_shall_stop = false;
    m_has_stopped = true;
}

