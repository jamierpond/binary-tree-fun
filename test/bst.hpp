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

  auto print() const {
    Utils::print(*this);
  }

  struct Node {
    T value{};
    DataStruct data{};
    Node *left = nullptr;
    Node *right = nullptr;

    constexpr static auto noop = [](auto *) {};
    constexpr auto min() { return traversal::extreme_left(this, noop); }
    constexpr auto max() { return traversal::extreme_right(this, noop); }

    constexpr void remove (T value, Node *root= nullptr) {
      auto [node, parent] = traversal::find<T>(this, value, root);
      if (!node) { return; }
      auto left_exists = node->left != nullptr;
      auto right_exists = node->right != nullptr;
      auto has_both_children = left_exists && right_exists;
      bool we_are_left = [&] {
        if (parent->left) {
          return parent->left->value == node->value;
        }
        return false;
      }();
      auto& our_ptr_on_parent = we_are_left ? parent->left : parent->right;

      if (has_both_children) {
        auto *min = node->right->min();
        node->value = min->value;
        node->right->remove(min->value, node);
      } else if (left_exists) {
        our_ptr_on_parent = node->left;
        delete node;
      } else if (right_exists) {
        our_ptr_on_parent = node->right;
        delete node;
      } else {
        our_ptr_on_parent = nullptr;
        delete node;
      }
    }

    constexpr Node *find(T value) {
      constexpr auto noop = [](auto *, auto *) {};
      return traversal::find<T, Node>(this, value, nullptr).node;
    }

    // make this configurable with static lambda, to evaluate diff types?
    constexpr auto should_step_left(T new_value) noexcept {
      return new_value < value;
    }

    constexpr Node* insert(T new_value, Node *& root) {
      auto [node, parent] = traversal::find<T>(this, new_value, root);
      if (node) { return node; }
      if (!parent) { parent = root; }
      auto *new_node = new Node{new_value};
      if (should_step_left(new_value)) {
        parent->left = new_node;
      } else {
        parent->right = new_node;
      }
      return new_node;
    }
  };

  constexpr void insert(T value) {
    if (!root) {
      root = new Node(value);
      return;
    }
    root->insert(value, root);
    Utils::debug_tree_check(*this);
  }

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
    Utils::debug_tree_check(*this);
  }

};

