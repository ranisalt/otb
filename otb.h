#pragma once

#include <boost/iostreams/device/mapped_file.hpp>
#include <vector>

namespace otb {

using mapped_file = boost::iostreams::mapped_file_source;
using iterator = mapped_file::iterator;

namespace detail {

constexpr char ESCAPE = '\xFD';
constexpr char START = '\xFE';
constexpr char END = '\xFF';

} // namespace detail

struct node {
  node(char type, iterator props_begin) : props_begin{props_begin}, props_end{}, type{type} {}

  std::vector<node> children = {};
  iterator props_begin, props_end;
  char type;
};

class OTB {
public:
  OTB(const mapped_file &file, node root) : file{file}, root{std::move(root)} {}

  const auto &children() const { return root.children; }
  const auto &begin() const { return root.props_begin; }
  const auto &end() const { return root.props_end; }

private:
  mapped_file file;
  node root;
};

OTB load(const std::string &filename, const std::string &accepted_identifier);

} // namespace otb
