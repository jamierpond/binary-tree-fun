#pragma once
#include <array>
#include <vector>

namespace pond {

// you may not want to pay the performance cost of bounds checking
// due to the extra branching.
enum class BoundsChecking {
  Disabled, // "Because this is C++" -- John Kalb
  SilentReturn,
  // ThrowException, // todo
};

/* approximates std::deque, but with a constant allocated size */
template <typename T, size_t MaxSize, BoundsChecking BoundsCheckOption = BoundsChecking::Disabled>
struct deque {
  constexpr auto pop_back() { back_ptr = decrement(back_ptr); }
  constexpr auto pop_front() { front_ptr = increment(front_ptr); }
  constexpr auto push_front(T val) { emplace<decrement>(front_ptr, val); }
  constexpr auto push_back(T val) { emplace<increment>(back_ptr, val); }

  constexpr auto operator[](size_t i) const { return arr[(front_ptr + i) % MaxSize]; }
  constexpr auto front() const { return arr[front_ptr]; }
  constexpr auto back() const {
    if (size() <= 1) { return arr[front_ptr]; }
    return arr[decrement(back_ptr)];
  }

  constexpr auto size() const { return modulo(back_ptr - front_ptr); }
  constexpr auto empty() const { return front_ptr == back_ptr; }

private:
  template <auto PointerFn>
  constexpr auto emplace(size_t& ptr, T val) {
    constexpr auto sr = BoundsCheckOption == BoundsChecking::SilentReturn;
    if constexpr (sr) {
      if (nearly_full()) { return; }
    }
    arr[ptr] = val;
    ptr = PointerFn(ptr);
  }
  constexpr static auto modulo(size_t val) { return (val + MaxSize) % MaxSize; }
  constexpr static auto increment(size_t ptr) { return modulo(ptr + 1); }
  constexpr static auto decrement(size_t ptr) { return modulo(ptr - 1); }
  constexpr auto nearly_full() const { return increment(back_ptr) == front_ptr; }
  std::array<T, MaxSize> arr{};
  size_t front_ptr = 0;
  size_t back_ptr = 0;
};

} // namespace pond

constexpr auto get_max_pool_out_size = [](auto N, auto K) {
  return N - K + 1;
};

template <typename T, size_t K>
constexpr auto max_pool_1d(const T* arr, size_t N)  {
  auto sz = get_max_pool_out_size(N, K);
  // todo, make std::array and/or do in place
  // is more annoying to compare static std::array than std::vector
  // the operator== defined for std::array is not constexpr :(
  auto result = std::vector<T>(sz);

  auto dq = pond::deque<size_t, K + 1>{};

  for (auto i = 0; i < N; i += 1) {

    auto initial_window = i < K - 1;
    if (!initial_window && !dq.empty() && dq.front() < i - K + 1) {
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

template <typename T, size_t N, size_t K>
constexpr auto max_pool_1d(const std::array<T, N>& arr) {
  return max_pool_1d<T, K>(arr.data(), N);
}

static_assert(max_pool_1d<int, 8, 3>({8, 3, -1, -3, 5, 3, 6, 7}) == std::vector<int>{8, 3, 5, 5, 6, 7});
static_assert(max_pool_1d<int, 8, 2>({8, 3, -1, -3, 5, 3, 6, 7}) == std::vector<int>{8, 3, -1, 5, 5, 6, 7});
static_assert(max_pool_1d<int, 8, 5>({8, 3, -1, -3, 5, 3, 6, 7}) == std::vector<int>{8, 5, 6, 7});

