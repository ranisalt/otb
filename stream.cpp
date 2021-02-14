#include "stream.h"

#include <iostream>

std::string read_string(otb::iterator &first, const otb::iterator &last, int len) {
  if (last - first < len) {
    throw std::invalid_argument("Not enough bytes to read as string.");
  }

  std::string out;
  out.reserve(len);

  auto start = first, end = first + len;
  while (start < end) {
    if (*start == otb::detail::ESCAPE)
      ++start, ++end;
    out.push_back(*start);
    ++start;
  }

  first = end;
  return out;
}

void skip(otb::iterator &first, const otb::iterator &last, const int len) {
  if (last - first < len) {
    throw std::invalid_argument("Not enough bytes to skip.");
  }

  auto start = first, end = first + len;
  while (start < end) {
    if (*start == otb::detail::ESCAPE)
      ++start, ++end;
    ++start;
  }

  first = end;
}