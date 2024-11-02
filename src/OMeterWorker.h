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


#ifndef GTKMM_EXAMPLEWORKER_H
#define GTKMM_EXAMPLEWORKER_H

#include "config.h"
#include <gtkmm.h>
#include <thread>
#include <mutex>
#include <alsa/asoundlib.h>

#ifdef HAVE_OSC
#include <lo/lo.h>

typedef struct {
    int client_index;
    char* path;
    lo_message data;
} osc_message;
#endif

#define MAX_OSC_CLIENTS 16

class OMainWnd;

class OMeterWorker {
public:
    OMeterWorker();

    // Thread function.
    void do_work(OMainWnd* caller);

    void get_data(double* fraction_done, Glib::ustring* message) const;
    void stop_work();
    bool has_stopped() const;

#ifdef HAVE_OSC
    lo_address osc_client[MAX_OSC_CLIENTS];
    lo_server_thread osc_server;
    lo_server osc_server_out;
    const int new_osc_client(lo_message client);
    const int osc_client_exists(lo_message client);
    void send_osc(int client_index, const char* path, lo_message msg);
    void send_osc_all(const char* path, lo_message msg);
    void dump_message(const char* path, const char *types, lo_arg ** argv, int argc);
#endif

    Gtk::Window* m_caller;

private:
    // Synchronizes access to member data.
    mutable std::mutex m_Mutex;

    snd_ctl_elem_id_t *id;
    snd_hctl_elem_t *elem;
    
    // Data used by both GUI thread and worker thread.
    bool m_shall_stop;
    bool m_has_stopped;
    double m_fraction_done;
//    Glib::ustring m_message;


};

#endif // GTKMM_EXAMPLEWORKER_H

