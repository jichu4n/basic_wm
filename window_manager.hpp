/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *  Copyright (C) 2013-2017 Chuan Ji <ji@chu4n.com>                          *
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

#ifndef WINDOW_MANAGER_HPP
#define WINDOW_MANAGER_HPP

extern "C" {
#include <X11/Xlib.h>
}
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "util.hpp"

// Implementation of a window manager for an X screen.
class WindowManager {
 public:
  // Creates a WindowManager instance for the X display/screen specified by the
  // argument string, or if unspecified, the DISPLAY environment variable. On
  // failure, returns nullptr.
   static ::std::unique_ptr<WindowManager> Create(
      const std::string& display_str = std::string());

  ~WindowManager();

  // The entry point to this class. Enters the main event loop.
  void Run();

 private:
  // Invoked internally by Create().
  WindowManager(Display* display);
  // Frames a top-level window.
  void Frame(Window w, bool was_created_before_window_manager);
  // Unframes a client window.
  void Unframe(Window w);

  // Event handlers.
  void OnCreateNotify(const XCreateWindowEvent& e);
  void OnDestroyNotify(const XDestroyWindowEvent& e);
  void OnReparentNotify(const XReparentEvent& e);
  void OnMapNotify(const XMapEvent& e);
  void OnUnmapNotify(const XUnmapEvent& e);
  void OnConfigureNotify(const XConfigureEvent& e);
  void OnMapRequest(const XMapRequestEvent& e);
  void OnConfigureRequest(const XConfigureRequestEvent& e);
  void OnButtonPress(const XButtonEvent& e);
  void OnButtonRelease(const XButtonEvent& e);
  void OnMotionNotify(const XMotionEvent& e);
  void OnKeyPress(const XKeyEvent& e);
  void OnKeyRelease(const XKeyEvent& e);

  // Xlib error handler. It must be static as its address is passed to Xlib.
  static int OnXError(Display* display, XErrorEvent* e);
  // Xlib error handler used to determine whether another window manager is
  // running. It is set as the error handler right before selecting substructure
  // redirection mask on the root window, so it is invoked if and only if
  // another window manager is running. It must be static as its address is
  // passed to Xlib.
  static int OnWMDetected(Display* display, XErrorEvent* e);
  // Whether an existing window manager has been detected. Set by OnWMDetected,
  // and hence must be static.
  static bool wm_detected_;
  // A mutex for protecting wm_detected_. It's not strictly speaking needed as
  // this program is single threaded, but better safe than sorry.
  static ::std::mutex wm_detected_mutex_;

  // Handle to the underlying Xlib Display struct.
  Display* display_;
  // Handle to root window.
  const Window root_;
  // Maps top-level windows to their frame windows.
  ::std::unordered_map<Window, Window> clients_;

  // The cursor position at the start of a window move/resize.
  Position<int> drag_start_pos_;
  // The position of the affected window at the start of a window
  // move/resize.
  Position<int> drag_start_frame_pos_;
  // The size of the affected window at the start of a window move/resize.
  Size<int> drag_start_frame_size_;

  // Atom constants.
  const Atom WM_PROTOCOLS;
  const Atom WM_DELETE_WINDOW;
};

#endif
