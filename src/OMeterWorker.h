/* gtkmm example Copyright (C) 2013 gtkmm development team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTKMM_EXAMPLEWORKER_H
#define GTKMM_EXAMPLEWORKER_H

#include <gtkmm.h>
#include <thread>
#include <mutex>

class OMainWnd;

class OMeterWorker
{
public:
  OMeterWorker();

  // Thread function.
  void do_work(OMainWnd* caller);

  void get_data(double* fraction_done, Glib::ustring* message) const;
  void stop_work();
  bool has_stopped() const;

private:
  // Synchronizes access to member data.
  mutable std::mutex m_Mutex;

  // Data used by both GUI thread and worker thread.
  bool m_shall_stop;
  bool m_has_stopped;
  double m_fraction_done;
  Glib::ustring m_message;
};

#endif // GTKMM_EXAMPLEWORKER_H

