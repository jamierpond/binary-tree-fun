#pragma once
#include "tree_utils.hpp"
#include "tree_traversal.hpp"

struct NoData {};

template <typename T, typename DataStruct = NoData> struct BinarySearchTree {
  constexpr BinarySearchTree() = default;
  constexpr BinarySearchTree(T value) : root(new Node{value}) {}
  constexpr ~BinarySearchTree() {
      traversal::post_order(root, [](auto *n) {
          delete n;
      });
  }

  typedef TreeUtils<BinarySearchTree<T>> Utils;

  struct Node {
    T value{};
    DataStruct data{};
    Node *left = nullptr;
    Node *right = nullptr;

    constexpr static auto noop = [](auto *) {};
    constexpr auto min() { return traversal::extreme_left(this, noop); }
    constexpr auto max() { return traversal::extreme_right(this, noop); }

    constexpr void remove (T value, Node *parent = nullptr) {
      constexpr auto on_found = [](auto *& n, auto *& direct_parent) {
        auto left_exists = n->left != nullptr;
        auto right_exists = n->right != nullptr;
        auto has_both_children = left_exists && right_exists;
        bool we_are_left = [&] {
          if (direct_parent->left) {
            return direct_parent->left->value == n->value;
          }
          return false;
        }();
        auto& our_ptr_on_parent = we_are_left ? direct_parent->left : direct_parent->right;

        if (has_both_children) {
          auto *min = n->right->min();
          n->value = min->value;
          n->right->remove(min->value, n);
        } else if (left_exists) {
          our_ptr_on_parent = n->left;
          delete n;
        } else if (right_exists) {
          our_ptr_on_parent = n->right;
          delete n;
        } else {
          our_ptr_on_parent = nullptr;
          delete n;
        }
      };
      traversal::find<T>(this, value, parent, on_found);
    }

    constexpr Node *find(T value) {
      constexpr auto noop = [](auto *, auto *) {};
      return traversal::find<T, Node>(this, value, nullptr, noop);
    }

    // make this configurable with static lambda, to evaluate diff types?
    constexpr auto should_step_left(T new_value) noexcept {
      return new_value < value;
    }

    constexpr void insert(T new_value) {
      if (new_value == value) { return; }
      auto &side = should_step_left(new_value) ? left : right;
      if (side) {
        side->insert(new_value);
      } else {
        side = new Node{new_value};
      }
    }
  };

  Node *root = nullptr;

  constexpr Node *find(T value) {
    if (!root) { return nullptr; }
    return root->find(value);
  }

  constexpr Node *min() const {
    if (!root) { return nullptr; }
    return root->min();
  }

  constexpr Node *max() const {
    if (!root) { return nullptr; }
    return root->max();
  }

  constexpr auto remove(T value) {
    if (!root) { return; }
    root->remove(value, root);
    // Utils::debug_tree_check(*this);
  }

  constexpr auto insert(T value) {
    if (!root) {
      root = new Node(value);
      return;
    }
    root->insert(value);
    // Utils::debug_tree_check(*this);
  }
};

