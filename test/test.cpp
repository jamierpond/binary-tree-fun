#include "catch2/catch_test_macros.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

template <typename T, size_t MaxSize = 1024>
struct BinaryTree {
  // todo static assert comparable
  // todo data orient these so pointers are iterated together and THEN
  // you look up the node value
  struct Node {
    T value{};
    Node *left = nullptr;
    Node *right = nullptr;
  };

  constexpr auto is_balanced() { return true; };

  constexpr auto insert(T val) {
    auto* r = root();
    if (r != nullptr) {
      r->value = val;
    }
    assert(false);
    // go down the tre
  };

  auto root() -> Node* {
    auto* r = data[0];
    if (r != nullptr) { return r; }
    r = new Node();
    return r;
  };

  std::array<Node*, MaxSize> data{};
};

TEST_CASE("insertion works") {
  auto tree = BinaryTree<int>{};
  REQUIRE(tree.data[0] == nullptr);
  tree.insert(5);

  REQUIRE(tree.root()->value == 5);
};

TEST_CASE("Hello, World!") { REQUIRE(1 == 1); }
