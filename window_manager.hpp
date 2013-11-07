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

#ifndef XON_WINDOW_MANAGER_HPP
#define XON_WINDOW_MANAGER_HPP

#include <list>
#include <memory>
#include <string>
extern "C" {
#include <X11/Xlib.h>
}

namespace xon {

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

  // Handle to the underlying Xlib Display struct.
  Display* display_;
  // Handle to root window.
  const Window root_;
};

}

#endif
