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

#include "client.hpp"
#include <gdk/gdkx.h>
#include <glog/logging.h>

using ::Glib::RefPtr;

Client::Client(RefPtr<::Gdk::Display> g_display, Window w, Window frame)
    : g_display_(g_display),
      display_(GDK_DISPLAY_XDISPLAY(g_display_->gobj())),
      g_cursor_(::Gdk::Cursor::create(::Gdk::LEFT_PTR)),
      w_(w),
      frame_(frame),
      g_frame_(::Glib::wrap(
          gdk_x11_window_foreign_new_for_display(
              g_display_->gobj(), frame_))),
      g_header_(::Gtk::WINDOW_POPUP),
      g_header_grid_(),
      g_label_("Hello!"),
      g_close_button_("X"),
      is_dragging_(false) {
  g_header_.add(g_header_grid_);
  g_label_.set_hexpand(true);
  g_label_.set_vexpand(true);
  g_label_.set_halign(::Gtk::Align::ALIGN_FILL);
  g_label_.set_valign(::Gtk::Align::ALIGN_FILL);
  g_label_.set_alignment(::Gtk::ALIGN_START, ::Gtk::ALIGN_CENTER);
  g_label_.override_background_color(::Gdk::RGBA("#ff0000"));
  g_label_.override_color(::Gdk::RGBA("#ffffff"));
  g_header_grid_.attach(g_label_, 0, 0, 1, 1);
  g_close_button_.set_vexpand(true);
  g_close_button_.set_halign(::Gtk::Align::ALIGN_FILL);
  g_close_button_.set_valign(::Gtk::Align::ALIGN_FILL);
  g_header_grid_.attach(g_close_button_, 1, 0, 1, 1);
  g_header_.resize(g_frame_->get_width(), HEADER_HEIGHT);
  g_header_.show_all_children();
  g_header_.show();

  RefPtr<::Gdk::Window> g_header_window = g_header_.get_window();
  Window header = GDK_WINDOW_XID(g_header_window->gobj());
  XReparentWindow(display_, header, frame, 0, 0);

  g_frame_->set_cursor(g_cursor_);

  g_header_.add_events(
      ::Gdk::BUTTON_PRESS_MASK |
      ::Gdk::BUTTON_MOTION_MASK |
      ::Gdk::BUTTON_RELEASE_MASK);
  g_header_.signal_button_press_event().connect(
      ::sigc::mem_fun(this, &Client::OnTitleButtonPress));
  g_header_.signal_motion_notify_event().connect(
      ::sigc::mem_fun(this, &Client::OnTitleMotionNotify));
  g_header_.signal_button_release_event().connect(
      ::sigc::mem_fun(this, &Client::OnTitleButtonRelease));
}

bool Client::OnTitleButtonPress(GdkEventButton* e) {
  if (e->button == 1) {
    LOG(INFO) << "Press";
    is_dragging_ = true;
    drag_start_pos_ = Position<int>(e->x_root, e->y_root);
    g_frame_->get_root_origin(drag_start_frame_pos_.x, drag_start_frame_pos_.y);
  }
  g_frame_->raise();
  return false;
}

bool Client::OnTitleMotionNotify(GdkEventMotion* e) {
  LOG(INFO) << "Motion";
  if (!is_dragging_) {
    return false;
  }
  g_frame_->raise();
  const Position<int> drag_pos(e->x_root, e->y_root);
  const Vector2D<int> delta = drag_pos - drag_start_pos_;
  const Position<int> dest_frame_pos = drag_start_frame_pos_ + delta;
  g_frame_->move(dest_frame_pos.x, dest_frame_pos.y);
  return true;
}

bool Client::OnTitleButtonRelease(GdkEventButton* e) {
  LOG(INFO) << "Release";
  is_dragging_ = false;
  return true;
}

