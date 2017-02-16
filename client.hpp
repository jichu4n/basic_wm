/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *  Copyright (C) 2017 Chuan Ji <ji@chu4n.com>                               *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *   http://www.apache.org/licenses/LICENSE-2.0                              *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <memory>
#include <gtkmm.h>
extern "C" {
#include <X11/Xlib.h>
}
#include "util.hpp"

// A Client object manages a client window, its frame window and decorations.
class Client {
  public:

    // Height of frame title bar.
    static const int HEADER_HEIGHT = 30;

    Client(::Glib::RefPtr<::Gdk::Display> g_display, Window w, Window frame);

    Window const GetFrame() { return frame_; }

  private:
    bool OnTitleButtonPress(GdkEventButton* e);
    bool OnTitleMotionNotify(GdkEventMotion* e);
    bool OnTitleButtonRelease(GdkEventButton* e);

    ::Glib::RefPtr<::Gdk::Display> g_display_;
    Display* display_;
    ::Glib::RefPtr<::Gdk::Cursor> g_cursor_;
    Window w_, frame_;
    ::Glib::RefPtr<::Gdk::Window> g_frame_;
    ::Gtk::Window g_header_;
    ::Gtk::Grid g_header_grid_;
    ::Gtk::Label g_label_;
    ::Gtk::Button g_close_button_;

    Position<int> drag_start_pos_;
    Position<int> drag_start_frame_pos_;
    bool is_dragging_;
};

#endif

