#include "catch2/catch_test_macros.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

template <typename T> struct Node {
  T value{};
  Node *left = nullptr;
  Node *right = nullptr;
  constexpr auto should_step_left(T new_value) const noexcept {
    return new_value < value;
  }
  constexpr auto insert_below(T new_value) {
    auto insertion = [](auto *side, auto value) {
      if (side) {
        side->insert_below(value);
      } else {
        side = new Node{value};
      }
    };

    if (should_step_left(new_value)) {
      insertion(left);
    } else {
      insertion(right);
    }
  }
};

template <typename T, size_t MaxSize = 1024> struct BinaryTree {
  struct Node {
    T value{};
    Node *left = nullptr;
    Node *right = nullptr;
    // make this configurable with static lambda ?
    constexpr auto should_step_left(T new_value) const noexcept {
      return new_value < value;
    }

    constexpr void insert_below(T new_value) {
      auto insertion = [](auto &side, auto value) {
        if (side) {
          side->insert_below(value);
        } else {
          side = new Node{value};
        }
      };

      if (should_step_left(new_value)) {
        insertion(left, new_value);
      } else {
        insertion(right, new_value);
      }
    }
  };

  Node *root = nullptr;

  auto insert(T value) {
    if (!root) {
      root = new Node(value);
      return;
    }
    root->insert_below(value);
  }

  ~BinaryTree() {
    for (auto *node : data) {
      if (node != nullptr) {
        delete node;
      }
    }
  }

  std::array<Node *, MaxSize> data{};
};

TEST_CASE("right insertion works") {
  auto tree = BinaryTree<int>{};
  REQUIRE(tree.data[0] == nullptr);
  tree.insert(0);
  auto* n = tree.root;
  for (int i = 1; i < 100; i++) {
    tree.insert(i);
    n = n->right;
    REQUIRE(n->value == i);
  }
};

TEST_CASE("left insertion works") {
  auto tree = BinaryTree<int>{};
  REQUIRE(tree.data[0] == nullptr);
  tree.insert(100);
  auto* n = tree.root;
  for (int i = 99; i <= 0; i--) {
    tree.insert(i);
    n = n->left;
    REQUIRE(n->value == i);
  }
};

TEST_CASE("Hello, World!") { REQUIRE(1 == 1); }
