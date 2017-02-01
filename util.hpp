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

#ifndef UTIL_HPP
#define UTIL_HPP

extern "C" {
#include <X11/Xlib.h>
}
#include <ostream>
#include <string>

// Represents a 2D size.
template <typename T>
struct Size {
  T width, height;

  Size() = default;
  Size(T w, T h)
      : width(w), height(h) {
  }

  ::std::string ToString() const;
};

// Outputs a Size<T> as a string to a std::ostream.
template <typename T>
::std::ostream& operator << (::std::ostream& out, const Size<T>& size);


// Represents a 2D position.
template <typename T>
struct Position {
  T x, y;

  Position() = default;
  Position(T _x, T _y)
      : x(_x), y(_y) {
  }

  ::std::string ToString() const;
};

// Represents a 2D vector.
template <typename T>
struct Vector2D {
  T x, y;

  Vector2D() = default;
  Vector2D(T _x, T _y)
      : x(_x), y(_y) {
  }

  ::std::string ToString() const;
};

// Outputs a Size<T> as a string to a std::ostream.
template <typename T>
::std::ostream& operator << (::std::ostream& out, const Position<T>& pos);

// Position operators.
template <typename T>
Vector2D<T> operator - (const Position<T>& a, const Position<T>& b);
template <typename T>
Position<T> operator + (const Position<T>& a, const Vector2D<T> &v);
template <typename T>
Position<T> operator + (const Vector2D<T> &v, const Position<T>& a);
template <typename T>
Position<T> operator - (const Position<T>& a, const Vector2D<T> &v);

// Size operators.
template <typename T>
Vector2D<T> operator - (const Size<T>& a, const Size<T>& b);
template <typename T>
Size<T> operator + (const Size<T>& a, const Vector2D<T> &v);
template <typename T>
Size<T> operator + (const Vector2D<T> &v, const Size<T>& a);
template <typename T>
Size<T> operator - (const Size<T>& a, const Vector2D<T> &v);

// Joins a container of elements into a single string, with elements separated
// by a delimiter. Any element can be used as long as an operator << on ostream
// is defined.
template <typename Container>
::std::string Join(const Container& container, const ::std::string& delimiter);

// Joins a container of elements into a single string, with elements separated
// by a delimiter. The elements are converted to string using a converter
// function.
template <typename Container, typename Converter>
::std::string Join(
    const Container& container,
    const ::std::string& delimiter,
    Converter converter);

// Returns a string representation of a built-in type that we already have
// ostream support for.
template <typename T>
::std::string ToString(const T& x);

// Returns a string describing an X event for debugging purposes.
extern ::std::string ToString(const XEvent& e);

// Returns a string describing an X window configuration value mask.
extern ::std::string XConfigureWindowValueMaskToString(unsigned long value_mask);

// Returns the name of an X request code.
extern ::std::string XRequestCodeToString(unsigned char request_code);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                               IMPLEMENTATION                              *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <algorithm>
#include <vector>
#include <sstream>

template <typename T>
::std::string Size<T>::ToString() const {
  ::std::ostringstream out;
  out << width << 'x' << height;
  return out.str();
}

template <typename T>
::std::ostream& operator << (::std::ostream& out, const Size<T>& size) {
  return out << size.ToString();
}

template <typename T>
::std::string Position<T>::ToString() const {
  ::std::ostringstream out;
  out << "(" << x << ", " << y << ")";
  return out.str();
}

template <typename T>
::std::ostream& operator << (::std::ostream& out, const Position<T>& size) {
  return out << size.ToString();
}

template <typename T>
::std::string Vector2D<T>::ToString() const {
  ::std::ostringstream out;
  out << "(" << x << ", " << y << ")";
  return out.str();
}

template <typename T>
::std::ostream& operator << (::std::ostream& out, const Vector2D<T>& size) {
  return out << size.ToString();
}

template <typename T>
Vector2D<T> operator - (const Position<T>& a, const Position<T>& b) {
  return Vector2D<T>(a.x - b.x, a.y - b.y);
}

template <typename T>
Position<T> operator + (const Position<T>& a, const Vector2D<T> &v) {
  return Position<T>(a.x + v.x, a.y + v.y);
}

template <typename T>
Position<T> operator + (const Vector2D<T> &v, const Position<T>& a) {
  return Position<T>(a.x + v.x, a.y + v.y);
}

template <typename T>
Position<T> operator - (const Position<T>& a, const Vector2D<T> &v) {
  return Position<T>(a.x - v.x, a.y - v.y);
}

template <typename T>
Vector2D<T> operator - (const Size<T>& a, const Size<T>& b) {
  return Vector2D<T>(a.width - b.width, a.height - b.height);
}

template <typename T>
Size<T> operator + (const Size<T>& a, const Vector2D<T> &v) {
  return Size<T>(a.width + v.x, a.height + v.y);
}

template <typename T>
Size<T> operator + (const Vector2D<T> &v, const Size<T>& a) {
  return Size<T>(a.width + v.x, a.height + v.y);
}

template <typename T>
Size<T> operator - (const Size<T>& a, const Vector2D<T> &v) {
  return Size<T>(a.width - v.x, a.height - v.y);
}

template <typename Container>
::std::string Join(const Container& container, const ::std::string& delimiter) {
  ::std::ostringstream out;
  for (auto i = container.cbegin(); i != container.cend(); ++i) {
    if (i != container.cbegin()) {
      out << delimiter;
    }
    out << *i;
  }
  return out.str();
}

template <typename Container, typename Converter>
::std::string Join(
    const Container& container,
    const ::std::string& delimiter,
    Converter converter) {
  ::std::vector<::std::string> converted_container(container.size());
  ::std::transform(
      container.cbegin(),
      container.cend(),
      converted_container.begin(),
      converter);
  return Join(converted_container, delimiter);
}

template <typename T>
::std::string ToString(const T& x) {
  ::std::ostringstream out;
  out << x;
  return out.str();
}

#endif
