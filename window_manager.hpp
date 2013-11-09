/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *  Copyright (C) 2013 Chuan Ji <jichuan89@gmail.com>                        *
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

#include <list>
#include <memory>
#include <string>
#include <unordered_map>
extern "C" {
#include <X11/Xlib.h>
}
#include "util.hpp"

// Implementation of a window manager for an X screen.
class WindowManager {
 public:
  // Creates a WindowManager instance for the X display/screen specified by the
  // argument string, or if unspecified, the DISPLAY environment variable. On
  // failure, returns nullptr.
  static std::unique_ptr<WindowManager> Create(
      const std::string& display_str = std::string());

  ~WindowManager();

  // The entry point to this class. Enters the main event loop, and returns only
  // when window management on this screen is terminated by the user.
  void Run();

 private:
  // Invoked internally by Create().
  WindowManager(Display* display);
  // Frames a top-level window.
  void Frame(Window w);
  // Unframes a (previously) top-level window.
  void Unframe(Window w);

  // Event handlers. These can be overridden by child classes.
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

  // Xlib error handler.
  static int OnXError(Display* display, XErrorEvent* e);

  // Handle to the underlying Xlib Display struct.
  Display* display_;
  // Handle to root window.
  const Window root_;
  // Maps top-level windows to their frame windows.
  std::unordered_map<Window, Window> clients_;
  // During a window move/resize, the location of the initial cursor position.
  Position<int> drag_start_pos_;
  // During a window move/resize, the location of the initial window position.
  Position<int> drag_start_frame_pos_;
  // During a window move/resize, the initial size of the window.
  Size<int> drag_start_frame_size_;

  // Atom constants.
  const Atom WM_PROTOCOLS, WM_DELETE_WINDOW;
};

#endif
