#include "catch2/catch_test_macros.hpp"
#include <cstdint>
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

namespace traversal {

template <typename Node, typename Callback>
constexpr void in_order (Node *node, Callback &&callback) {
  if (!node) { return; }
  in_order(node->left, callback);
  callback(node);
  in_order(node->right, callback);
};

template <typename Node, typename Callback>
constexpr void pre_order (Node *node, Callback &&callback) {
  if (!node) { return; }
  callback(node);
  pre_order(node->left, callback);
  pre_order(node->right, callback);
};

template <typename Node, typename Callback>
constexpr void level_order (Node *node, Callback &&callback) {
  if (!node) { return; }
  std::queue<Node *> q{};
  q.push(node);
  while (!q.empty()) {
    auto *n = q.front();
    q.pop();
    callback(n);
    if (n->left) { q.push(n->left); }
    if (n->right) { q.push(n->right); }
  }
};

template <typename Node, typename Callback>
constexpr void post_order (Node *node, Callback &&callback) {
  if (!node) { return; }
  post_order(node->left, callback);
  post_order(node->right, callback);
  callback(node);
};


template <typename Node, typename Callback>
constexpr Node* extreme(Node *node, Callback &&callback, auto get_next) {
  auto *n = node;
  while (get_next(n)) {
    callback(n);
    n = get_next(n);
  }
  return n;
};

template <typename Node, typename Callback>
constexpr Node* extreme_left(Node *node, Callback &&callback = [](auto*){}) {
    return extreme(node, callback, [](auto *n) { return n->left; });
};

template <typename Node, typename Callback>
constexpr Node* extreme_right(Node *node, Callback &&callback) {
    return extreme(node, callback, [](auto *n) { return n->right; });
};

template <typename T, typename Node, typename Callback>
Node *find(Node* root, T value, Node *parent = nullptr, Callback &&on_found_node = [](auto*&, auto*&) {}) {
  Node *n = root;
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

} // namespace traversal


struct NoData {};

#include <iostream>
template <typename T, typename DataStruct = NoData> struct BinaryTree {
  ~BinaryTree() {
      traversal::post_order(root, [](auto *n) {
          delete n;
      });
  }


  void print() {
    traversal::in_order(root, [](auto *n) {
      std::cout << n->value << std::endl;
    });
  }

  bool check_tree_valid() const /* throws */ {
    if (!root) { return true; }
    auto current = min()->value;
    try {
      traversal::in_order(root, [&](auto *n) {
        if (n->value < current) {
          throw std::runtime_error("tree is not valid");
        }
        current = n->value;
      });
    } catch (const std::runtime_error &e) {
      return false;
    }
    return true;
  }

  void print_smallest_to_biggest() {
    traversal::in_order(root, [](auto *n) { std::cout << n->value << std::endl; });
  }

  struct Node {
    T value{};
    DataStruct data{};
    Node *left = nullptr;
    Node *right = nullptr;

    constexpr static auto noop = [](auto *) {};
    auto min() { return traversal::extreme_left(this, noop); }
    auto max() { return traversal::extreme_right(this, noop); }

    void remove (T value, Node *parent = nullptr) {
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

    Node *find(T value) {
      constexpr auto noop = [](auto *, auto *) {};
      return traversal::find<T, Node>(this, value, nullptr, noop);
    }

    // make this configurable with static lambda ?
    constexpr auto should_step_left(T new_value) noexcept {
      return new_value < value;
    }

    void insert(T new_value) {
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

  Node *find(T value) {
    if (!root) { return nullptr; }
    return root->find(value);
  }

  Node *min() const {
    if (!root) { return nullptr; }
    return root->min();
  }

  Node *max() const {
    if (!root) { return nullptr; }
    return root->max();
  }

  auto debug_tree_check() {
#ifndef NDEBUG
    if (!check_tree_valid()) {
      std::cerr << "tree is not valid" << std::endl;
      std::abort();
    }
#endif
  }

  auto remove(T value) {
    if (!root) { return; }
    root->remove(value, root);
    debug_tree_check();
  }

  auto insert(T value) {
    if (!root) {
      root = new Node(value);
      return;
    }
    root->insert(value);
    debug_tree_check();
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

TEST_CASE("zero children node deletion") {
  auto tree = BinaryTree<int>{};
  tree.insert(1);
  tree.insert(2);
  tree.insert(3);
  tree.insert(4);

  REQUIRE(tree.root->right->value == 2);
  REQUIRE(tree.root->right->right->value == 3);
  REQUIRE(tree.root->right->right->right->value == 4);
  REQUIRE(tree.root->right->right->right != nullptr);

  tree.print();

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
  REQUIRE(tree.check_tree_valid());

  std::cout << "+++++ smallest to biggest +++++" << std::endl;
  tree.print_smallest_to_biggest();
};


auto fuzz = [](auto lower, auto upper) {
  auto tree = BinaryTree<int>{};
  uint64_t seed = 0;

  for (int i = 0; i < 1'000; i++) {
    auto next = rand() % (upper - lower) + lower;
    auto* found = tree.find(next);
    tree.insert(next);
  }

  // remove some random ones
  std::vector<BinaryTree<int>::Node*> to_remove{};
  traversal::in_order(tree.root, [&](auto *n) {
    if (rand() % 2) {
       to_remove.push_back(n);
    }
  });

  REQUIRE(tree.check_tree_valid());
};

TEST_CASE("random inserts") {
  fuzz(-20, 10);
  fuzz(0, 10);
  fuzz(0, 1);
  fuzz(10, -20);
  fuzz(1 << 1, 1 << 15);
};

TEST_CASE("Hello, World!") { REQUIRE(1 == 1); }
