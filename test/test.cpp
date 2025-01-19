#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <iostream>
#include <cstdint>
#include "bst.hpp"


TEST_CASE("extreme right insertion works") {
  auto tree = BinarySearchTree<int>{};
  tree.insert(0);
  auto *n = tree.root;
  for (int i = 1; i < 100; i++) {
    tree.insert(i);
    n = n->right;
    REQUIRE(n->value == i);
  }
};

TEST_CASE("extreme left insertion works") {
  auto tree = BinarySearchTree<int>{};
  tree.insert(100);
  auto *n = tree.root;
  for (int i = 99; i <= 0; i--) {
    tree.insert(i);
    n = n->left;
    REQUIRE(n->value == i);
  }
};

TEST_CASE("can alternate") {
  auto tree = BinarySearchTree<int>{};
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
  auto tree = BinarySearchTree<int>{};
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
  auto tree = BinarySearchTree<int>{};
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
  auto tree = BinarySearchTree<int>{};
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
  tree.print();
};


auto fuzz = [](auto lower, auto upper) {
  auto tree = BinarySearchTree<int>{};
  for (int i = 0; i < 1'000; i++) {
    auto next = rand() % (upper - lower) + lower;
    auto* found = tree.find(next);
    tree.insert(next);
  }

  // remove some random ones
  traversal::post_order(tree.root, [&](auto *n) {
    if (rand() % 2) {
      tree.remove(n->value);
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
