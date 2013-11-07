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

// Common utility classes and types.

#ifndef XON_TYPES_HPP
#define XON_TYPES_HPP

#include <string>
#include <ostream>

namespace xon {

// Represents a 2D size.
template <typename T>
struct Size {
  T width, height;

  Size() = default;
  Size(T w, T h)
      : width(w), height(h) {
  }

  std::string ToString() const;
};

// Outputs a Size<T> as a string to a std::ostream.
template <typename T>
std::ostream& operator << (std::ostream& out, const Size<T>& size);


// Represents a 2D position.
template <typename T>
struct Position {
  T x, y;

  Position() = default;
  Position(T _x, T _y)
      : x(_x), y(_y) {
  }

  std::string ToString() const;
};

// Outputs a Size<T> as a string to a std::ostream.
template <typename T>
std::ostream& operator << (std::ostream& out, const Position<T>& pos);

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               IMPLEMENTATION                              *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <sstream>

namespace xon {

template <typename T>
std::string Size<T>::ToString() const {
  std::ostringstream out;
  out << width << 'x' << height;
  return out.str();
}

template <typename T>
std::ostream& operator << (std::ostream& out, const Size<T>& size) {
  return out << size.ToString();
}

template <typename T>
std::string Position<T>::ToString() const {
  std::ostringstream out;
  out << "(" << x << ", " << y << ")";
  return out.str();
}

template <typename T>
std::ostream& operator << (std::ostream& out, const Position<T>& size) {
  return out << size.ToString();
}

}

#endif
