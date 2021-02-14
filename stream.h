#pragma once

#include "otb.h"

template <class T> T read(otb::iterator &first, const otb::iterator &last) {
  constexpr decltype(last - first) len = sizeof(T);

  std::string buf;
  buf.reserve(len);

  while (buf.size() < len and first < last) {
    if (*first == otb::detail::ESCAPE) {
      ++first;
    }
    buf.push_back(*first);
    ++first;
  }

  if (buf.size() < len) {
    throw std::invalid_argument("Not enough bytes to read.");
  }

  T out;
  std::copy(buf.begin(), buf.end(), reinterpret_cast<char *>(&out));
  return out;
}

std::string read_string(otb::iterator &first, const otb::iterator &last, int len);
void skip(otb::iterator &first, const otb::iterator &last, int len);
