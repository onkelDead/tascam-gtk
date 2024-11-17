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

#include "OMeterWorker.h"
#include "OMainWnd.h"
#include <sstream>
#include <chrono>


#ifdef HAVE_OSC
void osc_err_handler(int num, const char *msg, const char *where) {
    fprintf(stderr, "ARDMIX_ERROR %d: %s at %s\n", num, msg, where);
}

int osc_handler(const char *path, const char *types, lo_arg ** argv, int argc, lo_message data, void *user_data) {
    int ret;
	int client_index = -1;

	OMeterWorker* worker = (OMeterWorker*) user_data;


        worker->one_client = lo_address_get_url(lo_message_get_source(data));
        
	OMainWnd* wnd = ((OMainWnd*)(worker->m_caller));
	
	osc_message* msg = new osc_message;
	msg->path = strdup(path);
	msg->data = lo_message_clone(data);
	msg->client_index = client_index;
	
	wnd->oscMutex.lock();
	
	g_async_queue_push (wnd->m_osc_queue, msg);
	wnd->notify_osc();

	wnd->oscMutex.unlock();
	
    return 0;
}
#endif


OMeterWorker::OMeterWorker() :
m_Mutex(),
m_shall_stop(false),
m_has_stopped(false) {
#ifdef HAVE_OSC
    for( int i = 0; i < MAX_OSC_CLIENTS; i++ ) {
        osc_client[i] = NULL;
    }	
#endif
}


void OMeterWorker::stop_work() {
	std::lock_guard<std::mutex> lock(m_Mutex);
	m_shall_stop = true;
}

bool OMeterWorker::has_stopped() const {
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_has_stopped;
}


void OMeterWorker::do_work(OMainWnd* caller) {
	m_has_stopped = false;

	m_caller = caller;
	
#ifdef HAVE_OSC
    osc_server = lo_server_thread_new_with_proto("3135", LO_UDP, osc_err_handler);
    if( !osc_server ) {
        fprintf(stderr, "ERROR: unable to create client port.\n");
        return;
    }
    lo_server_thread_add_method(osc_server, NULL, NULL, osc_handler, this);
    lo_server_thread_start(osc_server);
#endif	
	
 
    elem = caller->alsa->getElement(CTL_NAME_METER);
    
    for (;;) // do until break
    {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if (m_shall_stop) {
                    break;
            }
            caller->alsa->getControlIntegers(elem, caller->alsa->meters, 34);
            caller->notify();
    }
#ifdef HAVE_OSC
lo_server_thread_free(osc_server);
#endif	

    m_shall_stop = false;
    m_has_stopped = true;

}

#ifdef HAVE_OSC
const int OMeterWorker::new_osc_client(const char* new_client_url) {
    int i;
    for(i = 0; i < MAX_OSC_CLIENTS; i++) {
        if( osc_client[i] == NULL) {
           osc_client[i] = strdup(new_client_url); 
           fprintf(stdout, "OSC-client %s\n", osc_client[i]);
           return i;
        }
    }
	return -1;
}


const int OMeterWorker::osc_client_exists(const char* new_client_url) {
    int i;
    for( i = 0; i < MAX_OSC_CLIENTS; i++ ) {
        if( osc_client[i] && !strcmp(new_client_url, osc_client[i]) ){
            return i;
        }
    }
    return -1;
}

void OMeterWorker::dump_message(const char* path, const char *types, lo_arg ** argv, int argc ) {
    int i;
    
    fprintf(stdout, "ardour path: <%s> - ", path);
    for (i = 0; i < argc; i++) {
        fprintf(stdout, " '%c':", types[i]);
        lo_arg_pp((lo_type) types[i], argv[i]);
    }
    fprintf(stdout, "\n");
    fflush(stdout);
    
}

void OMeterWorker::send_osc(int client_index, const char* path, lo_message msg)  {
	
	lo_send_message_from(lo_address_new_from_url(osc_client[client_index]), osc_server_out, path, msg);
	
}
void OMeterWorker::send_osc_all(const char* path, lo_message msg)  {
	
    if (one_client) {
        lo_address a = lo_address_new_from_url(one_client);
        lo_send_message(a, path, msg);
    }
}
#endif
