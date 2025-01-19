#pragma once
#include <array>
#include "bst.hpp"

template <typename T, std::size_t N, std::size_t WindowSize, std::size_t Stride>
constexpr auto max_pooling_1d (std::array<T, N> const &arr) {
  static_assert(N > 0, "N must be greater than 0");
  constexpr auto num_windows = N / Stride;
  constexpr auto LenOutput = num_windows * WindowSize;
  std::array<T, LenOutput> output{};
  auto tree = BinarySearchTree<T>{};
  for (std::size_t i = 0; i < N; i += Stride) {
    for (std::size_t j = 0; j < WindowSize; j++) {
      tree.insert(arr[i + j]);
    }
    output[i] = tree.root->value;
    tree.remove(arr[i]);
  }

  return output;
};

constexpr auto arr_equal = []<typename T, std::size_t N>(std::array<T, N> const &a, std::array<T, N> const &b) {
  for (std::size_t i = 0; i < N; i++) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
};


static_assert(arr_equal(max_pooling_1d<int, 2, 2, 1>({2, 1}), {2}));
