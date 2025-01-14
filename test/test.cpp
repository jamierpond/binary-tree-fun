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
    Node *left = nullptr;
    Node *right = nullptr;

    auto min() { return traverse_extreme([](auto *n) { return n->left; }); }
    auto max() { return traverse_extreme([](auto *n) { return n->right; }); }
    auto traverse_extreme(auto get_next) {
      auto *n = this;
      while (get_next(n)) { n = get_next(n); }
      return n;
    }

    void delete_recursive() {
      for (auto *n : {left, right}) {
        if (n) {
          n->delete_recursive();
          n = nullptr;
          delete n;
        }
      }
    }

    using FoundNodeCallback = std::function<void(Node *, Node *)>;
    Node *find(
        T value, FoundNodeCallback on_found_node = [](auto *, auto *) {},
        Node *parent = nullptr) {
      Node *n = this;
      for (;;) {
        if (n->value == value) {
          on_found_node(n, parent);
          return n;
        }
        auto *next = n->should_step_left(value) ? n->left : n->right;
        if (!next) {
          return nullptr;
        }
        parent = n;
        n = next;
      }
    }

    enum class ChildrenState {
      NONE = 0,
      BOTH,
      LEFT_ONLY,
      RIGHT_ONLY,
    };

    void remove(T value, Node *parent = nullptr) {
      FoundNodeCallback rm = [](Node *n, Node *parent) {
        auto we_are_left = [&]() {
          // we kind of guarantee the right should not be nullptr
          // since at this point every node has at least one child
          if (!parent->left) { return false; }
          return parent->left == n;
        }();

        switch (n->children_state()) {
        case Node::ChildrenState::NONE: {
          delete n;
          auto &side = we_are_left ? parent->left : parent->right;
          side = nullptr;
        } break;
        case Node::ChildrenState::RIGHT_ONLY:
          parent->right = n->right;
          delete n;
          break;
        case Node::ChildrenState::LEFT_ONLY:
          parent->left = n->left;
          delete n;
          break;
        case Node::ChildrenState::BOTH: {
          auto *right_min = n->right->min();
          auto right_min_value = right_min->value;
          n->value = right_min_value;
          n->right->remove(right_min_value, n);
        } break;
        default:
          // unreachable
          std::abort();
        }
      };

      find(value, rm, parent);
    }

    // make this configurable with static lambda ?
    constexpr auto should_step_left(T new_value) const noexcept {
      return new_value < value;
    }

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

    constexpr void insert(T new_value) {
      auto &side = should_step_left(new_value) ? left : right;
      if (side) {
        side->insert(new_value);
      } else {
        side = new Node{new_value};
      }
    }
  };

  Node *root = nullptr;

  Node *find(T value) {
    if (!root) { return nullptr; }
    return root->find(value);
  }

  Node *min() {
    if (!root) { return nullptr; }
    return root->min();
  }

  Node *max() {
    if (!root) { return nullptr; }
    return root->max();
  }

  auto remove(T value) {
    if (!root) { return; }
    root->remove(value, root);
  }

  auto insert(T value) {
    if (!root) {
      root = new Node(value);
      return;
    }
    root->insert(value);
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

TEST_CASE("delete child with two nodes") {
  auto tree = BinaryTree<int>{};
  // fork
  tree.insert(10);
  tree.insert(15);
  tree.insert(5);
  tree.insert(3);
  tree.insert(6);

  // REQUIRE(tree.root->left->left->left->value == 2);
  REQUIRE(tree.root->right->value == 15);

  auto five = tree.root->left;
  REQUIRE(five->value == 5);
  auto three = five->left;
  REQUIRE(three->value == 3);

  auto six = five->right;
  REQUIRE(six->value == 6);

  tree.remove(5);

  // ok so let's remove 5 whuic
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

  // test min and max
  REQUIRE(tree.min()->value == -3);
  REQUIRE(tree.max()->value == 4);
};

TEST_CASE("Hello, World!") { REQUIRE(1 == 1); }
