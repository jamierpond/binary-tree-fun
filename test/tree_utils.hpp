#pragma once
#include "tree_traversal.hpp"
#include <iostream>

template <typename Tree>
struct TreeUtils {
  static void print(Tree& tree) {
    traversal::in_order(tree.root, [](auto *n) {
      std::cout << n->value << std::endl;
    });
  }

  static bool check_tree_valid(const Tree& tree) /* throws */ {
    if (!tree.root) { return true; }
    auto current = tree.min()->value;
    auto max = tree.max()->value;
    try {
      traversal::in_order(tree.root, [&](auto *n) {
        if (n->value < current) {
          throw std::runtime_error("tree is not valid");
        }
        current = n->value;
      });
    } catch (const std::runtime_error &e) {
      return false;
    }
    if (current != max) {
      // if we didn't end up at the max value, the tree is invalid!
      return false;
    }
    return true;
  }

  static auto debug_tree_check(Tree& tree) {
#ifndef NDEBUG
    if (!check_tree_valid(tree)) {
      std::cerr << "tree is not valid" << std::endl;
      std::abort();
    }
#endif
  }
};

