#include "catch2/catch_test_macros.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

struct NoData {};

template <typename T, typename DataStruct = NoData> struct BinaryTree {
  ~BinaryTree() {
    if (root) {
      root->delete_recursive();
      delete root;
    }
  }

  struct Node {
    T value{};
    DataStruct data{};

    void delete_recursive() {
      for (auto* n : {left, right}) {
        if (n) {
          n->delete_recursive();
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

    enum class ChildrenState {
      NONE = 0,
      BOTH,
      LEFT_ONLY,
      RIGHT_ONLY,
    };

    ChildrenState children_state() const {
      auto l_present = left != nullptr;
      auto r_present = right != nullptr;
      if (l_present && r_present) {
        return ChildrenState::BOTH;
      } else if (l_present) {
        return ChildrenState::LEFT_ONLY;
      } else if (r_present) {
        return ChildrenState::RIGHT_ONLY;
      } else {
        return ChildrenState::NONE;
      }
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

  auto remove(T value) {
    auto rm = [](auto *n, auto *parent) {
      switch (n->children_state()) {
      case Node::ChildrenState::NONE:
        // was i left or right child ?
        delete n;
        if (parent->left == n) {
          parent->left = nullptr;
        } else {
          parent->right = nullptr;
        }
        break;
      case Node::ChildrenState::RIGHT_ONLY:
        parent->right = n->right;
        delete n;
        break;
      case Node::ChildrenState::LEFT_ONLY:
        parent->left = n->left;
        delete n;
        break;
      case Node::ChildrenState::BOTH:
        // more complex things
        // do the rotation
        break;
      }
    };

    find(value, rm);
  }

  using OnFoundNode = std::function<void(Node *, Node *)>;
  Node *find(
      T value, OnFoundNode &&on_found_node = [](auto *, auto *) {}) {

    if (!root) { return nullptr; }
    Node *n = root;
    Node *parent = nullptr;
    for (;;) {
      if (n->value == value) {
        on_found_node(n, parent);
        return n;
      }
      auto *next = n->should_step_left(value) ? n->left : n->right;
      if (!next) { return nullptr; }
      parent = n;
      n = next;
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

TEST_CASE("zeroo children node deletion") {
  auto tree = BinaryTree<int>{};
  tree.insert(1);
  tree.insert(2);
  tree.insert(3);
  tree.insert(4);

  REQUIRE(tree.root->right->value == 2);
  REQUIRE(tree.root->right->right->value == 3);
  REQUIRE(tree.root->right->right->right->value == 4);
  REQUIRE(tree.root->right->right->right != nullptr);
  REQUIRE(tree.root->right->right->right->children_state() ==
          BinaryTree<int>::Node::ChildrenState::NONE);

  tree.remove(4);

  REQUIRE(tree.root->right->right->right == nullptr);

  // add 4 back
  tree.insert(4);
  REQUIRE(tree.root->right->right->right->value == 4);
  REQUIRE(tree.root->right->right->right != nullptr);

  tree.remove(3);
  REQUIRE(tree.root->right->right->value == 4);

  tree.insert(-1);
  tree.insert(-3);
  tree.remove(-1);

  REQUIRE(tree.root->left->value == -3);
};

TEST_CASE("Hello, World!") { REQUIRE(1 == 1); }
