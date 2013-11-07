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

#ifndef XON_UTIL_HPP
#define XON_UTIL_HPP

#include <string>
extern "C" {
#include <X11/Xlib.h>
}

namespace xon {

// Joins a container of elements into a single string, with elements separated
// by a delimiter. Any element can be used as long as an operator << on ostream
// is defined.
template <typename Container>
std::string Join(const Container& container, const std::string& delimiter);

// Joins a container of elements into a single string, with elements separated
// by a delimiter. The elements are converted to string using a converter
// function.
template <typename Container, typename Converter>
std::string Join(
    const Container& container,
    const std::string& delimiter,
    Converter converter);

// Returns a string representation of a built-in type that we already have
// ostream support for.
template <typename T>
std::string ToString(const T& x);

// Returns a string describing an X event for debugging purposes.
extern std::string ToString(const XEvent& e);

// Returns a string describing an X window configuration value mask.
extern std::string XConfigureWindowValueMaskToString(unsigned long value_mask);

// Returns the name of an X request code.
extern std::string XRequestCodeToString(unsigned char request_code);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               IMPLEMENTATION                              *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <algorithm>
#include <vector>
#include <sstream>

namespace xon {

template <typename Container>
std::string Join(const Container& container, const std::string& delimiter) {
  std::ostringstream out;
  for (auto i = container.cbegin(); i != container.cend(); ++i) {
    if (i != container.cbegin()) {
      out << delimiter;
    }
    out << *i;
  }
  return out.str();
}

template <typename Container, typename Converter>
std::string Join(
    const Container& container,
    const std::string& delimiter,
    Converter converter) {
  std::vector<std::string> converted_container(container.size());
  std::transform(
      container.cbegin(),
      container.cend(),
      converted_container.begin(),
      converter);
  return Join(converted_container, delimiter);
}

template <typename T>
std::string ToString(const T& x) {
  std::ostringstream out;
  out << x;
  return out.str();
}

}

#endif
