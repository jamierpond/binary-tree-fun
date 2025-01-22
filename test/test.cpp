#include "catch2/catch_test_macros.hpp"
#include "tree_utils.hpp"
#include <cstddef>
#define CATCH_CONFIG_MAIN
#include "bst.hpp"
#include <catch2/catch_all.hpp>
#include <cstdint>
#include <iostream>

constexpr auto get_max_pool_out_size = [](auto N, auto K, auto Stride) {
  return (N / Stride) - K + 1;
};


/**

0 0 0 0 0
^
^

push_back(1)
1 0 0 0 0
  ^
^

pop_front()
1 0 0 0 0
  ^
  ^

push_back(2)
1 2 0 0 0
    ^
  ^

push_front(3)
3 2 0 0 0
    ^
^

pop_back()
3 2 0 0 0
  ^
^

//////////////////////////
///
push_front()
0 0 0 1 0
        ^
      ^

*/


namespace pond {
template <typename T, size_t MaxSize>
struct deque {
  constexpr auto operator[](size_t i) const { return arr[(front_ptr + i) % MaxSize]; }
  constexpr auto empty() const { return front_ptr == back_ptr; }
  constexpr auto pop_back() { back_ptr = decrement(back_ptr); }
  constexpr auto pop_front() { front_ptr = increment(front_ptr); }
  constexpr auto size() const { return modulo(back_ptr - front_ptr); }
  constexpr auto front() const { return arr[front_ptr]; }
  constexpr auto back() const {
    if (size() == 1) { return arr[front_ptr]; }
    return arr[decrement(back_ptr)];
  }

  constexpr auto push_back(T val) {
    arr[back_ptr] = val;
    back_ptr = increment(back_ptr);
  }

  constexpr auto push_front(T val) {
    arr[front_ptr] = val;
    front_ptr = decrement(front_ptr);
  }

private:
  std::array<T, MaxSize> arr{};
  size_t front_ptr = 0;
  size_t back_ptr = 0;
  constexpr static auto modulo(size_t val) { return (val + MaxSize) % MaxSize; }
  constexpr static auto increment(size_t ptr) { return modulo(ptr + 1); }
  constexpr static auto decrement(size_t ptr) { return modulo(ptr - 1); }
};
} // namespace pond

TEST_CASE("deque works") {
  auto dq = pond::deque<int, 10>{};
  REQUIRE(dq.empty());
  REQUIRE(dq.size() == 0);

  dq.push_back(1);
  dq.push_back(2);
  dq.push_back(3);
  dq.push_back(4);
  REQUIRE(dq.back() == 4);
  REQUIRE(dq.size() == 4);
  REQUIRE(dq.front() == 1);
  REQUIRE(!dq.empty());

  dq.pop_back();
  REQUIRE(dq.size() == 3);
  REQUIRE(dq.back() == 3);

  dq.pop_front();
  REQUIRE(dq.size() == 2);
  REQUIRE(dq.front() == 2);

  dq.pop_front();
  REQUIRE(dq.size() == 1);
  REQUIRE(dq.front() == 3);

  dq.pop_front();
  REQUIRE(dq.size() == 0);
  REQUIRE(dq.empty());
};

template <typename T, size_t K, size_t Stride = 1>
constexpr auto max_pool_1d(const T* arr, size_t N)  {
  auto dq = pond::deque<size_t, K + 1>{};
  auto sz = get_max_pool_out_size(N, K, Stride);
  using RT = std::vector<T>;
  auto result= RT(sz);

  for (int i = 0; i < N; i++) {
    auto initial_window = i < K - 1;
    if (not initial_window && !dq.empty() && dq.front() < i - K + 1) {
      dq.pop_front();
    }

    while (!dq.empty() && arr[dq.back()] < arr[i]) {
      dq.pop_back();
    }

    dq.push_back(i);

    if (i >= K - 1) {
      result[i - K + 1] = arr[dq.front()];
    }
  }
  return result;
}

template <typename T, size_t N, size_t K, size_t Stride>
constexpr auto max_pool_1d(const std::array<T, N>& arr) {
  return max_pool_1d<T, K, Stride>(arr.data(), N);
}

constexpr auto array_equal = [](auto a, auto b) {
  for (size_t i = 0; i < a.size(); i++) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
};

TEST_CASE("max pooling algo works") {
  auto arr = std::array<int, 8>{8, 3, -1, -3, 5, 3, 6, 7};
  auto expected = std::array<int, 6>{8, 3, 5, 5, 6, 7};
  auto out = max_pool_1d<int, 8, 3, 1>(arr);
  REQUIRE(array_equal(out, expected));
};

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
  REQUIRE(TreeUtils<BinarySearchTree<int>>::check_tree_valid(tree));

  std::cout << "+++++ smallest to biggest +++++" << std::endl;
};

auto fuzz = [](auto lower, auto upper) {
  auto tree = BinarySearchTree<int>{};
  std::int64_t seed = 0;

  for (int i = 0; i < 1'000; i++) {
    auto next = rand() % (upper - lower) + lower;
    auto *found = tree.find(next);
    tree.insert(next);
  }

  // remove some random ones
  traversal::post_order(tree.root, [&](auto *n) {
    if (rand() % 2) {
      tree.remove(n->value);
    }
  });

  REQUIRE(TreeUtils<BinarySearchTree<int>>::check_tree_valid(tree));
};

TEST_CASE("random inserts") {
  fuzz(-20, 10);
  fuzz(0, 10);
  fuzz(0, 1);
  fuzz(10, -20);
  fuzz(1 << 1, 1 << 15);
};

TEST_CASE("Hello, World!") { REQUIRE(1 == 1); }
