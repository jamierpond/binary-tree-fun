#include "catch2/catch_test_macros.hpp"
#include <initializer_list>
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>


template <typename T, size_t MaxSize = 1024> struct BinaryTree {
  struct Node {
    T value{};

    ~Node() {
      for (auto *n : std::initializer_list<Node *>{left, right}) {
        if (n) {
          delete n;
        }
      }
    }

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

  Node* find(T value) {
    if (!root) { return nullptr; }
    Node *n = root;
    for (;;) {
      if (n->value == value) { return n; }
      if (n->should_step_left(value)) {
        if (!n->left) { return nullptr; }
        n = n->left;
      } else {
        if (!n->right) { return nullptr; }
        n = n->right;
      }
    }
  }
};

TEST_CASE("extreme right insertion works") {
  auto tree = BinaryTree<int>{};
  tree.insert(0);
  auto *n = tree.root;
  for (int i = 1; i < 100; i++) {
    tree.insert(i);
    n = n->right;
    REQUIRE(n->value == i);
  }
};

TEST_CASE("extreme left insertion works") {
  auto tree = BinaryTree<int>{};
  tree.insert(100);
  auto *n = tree.root;
  for (int i = 99; i <= 0; i--) {
    tree.insert(i);
    n = n->left;
    REQUIRE(n->value == i);
  }
};

TEST_CASE("can alternate") {
  auto tree = BinaryTree<int>{};
  tree.insert(10);
  tree.insert(12);
  tree.insert(11);
  tree.insert(1100);
  tree.insert(-1);
  REQUIRE(tree.root->right->left->value == 11);
  REQUIRE(tree.root->right->right->value == 1100);
  REQUIRE(tree.root->left->value == -1);
};

TEST_CASE("can find") {
  auto tree = BinaryTree<int>{};
  tree.insert(10);
  tree.insert(12);
  tree.insert(11);
  tree.insert(110);

  REQUIRE(tree.find(110));
  REQUIRE(tree.find(11));
  REQUIRE(tree.find(0xfffff678) == nullptr);

  tree.insert(0xfffff678);
  REQUIRE(tree.find(0xfffff678));
};

TEST_CASE("Hello, World!") { REQUIRE(1 == 1); }
