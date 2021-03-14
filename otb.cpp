#include "otb.h"

#include <stack>
#include <string>
#include <string_view>

namespace otb {

namespace {

const auto wildcard_identifier = std::string_view{"\0\0\0\0", 4};

auto check_identifier(iterator it, std::string_view accepted_identifier) {
  auto identifier = std::string_view(it, 4);
  return identifier == accepted_identifier or identifier == wildcard_identifier;
}

auto parse_tree(iterator first, const iterator last) {
  if (*first != detail::START) {
    throw std::invalid_argument("Invalid first byte.");
  }

  ++first;
  auto root = node{*first, first + sizeof(node::type)};
  auto parse_stack = std::stack<node *, std::vector<node *>>{{&root}};

  auto get_current = [&]() -> node & {
    if (parse_stack.empty()) {
      throw std::invalid_argument("Parse stack is empty.");
    }
    return *parse_stack.top();
  };

  for (; first != last; ++first) {
    switch (*first) {
    case detail::START: {
      auto &node = get_current();
      if (node.children.empty()) {
        node.props_end = first;
      }
      if (++first == last) {
        throw std::invalid_argument("File overflow on start node.");
      }
      auto &child = node.children.emplace_back(*first, first + sizeof(node::type));
      parse_stack.push(&child);
      break;
    }
    case detail::END: {
      auto &node = get_current();
      if (node.children.empty()) {
        node.props_end = first;
      }
      parse_stack.pop();
      break;
    }
    case detail::ESCAPE:
      if (++first == last) {
        throw std::invalid_argument("File overflow on escape node.");
      }
      break;
    }
  }

  return root;
}

} // namespace

OTB load(const std::string &filename, std::string_view identifier) {
  auto file = mapped_file{filename};

  if (not check_identifier(file.begin(), identifier)) {
    throw std::invalid_argument("Invalid magic header.");
  }

  return {file, parse_tree(file.begin() + 4, file.end())};
}

} // namespace otb
