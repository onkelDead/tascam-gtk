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

OMeterWorker::OMeterWorker() :
m_Mutex(),
m_shall_stop(false),
m_has_stopped(false) {
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

	// Simulate a long calculation.
	for (;;) // do until break
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (m_shall_stop) {
			break;
		}
		caller->alsa->getIntegers("name='Z Meter'", caller->alsa->meters, 34);

		caller->notify();
	}

	m_shall_stop = false;
	m_has_stopped = true;

	caller->notify();
}
