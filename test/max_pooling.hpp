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

template <typename T, size_t MaxSize, BoundsChecking BoundsCheckOption = BoundsChecking::Disabled>
struct deque {
  constexpr auto pop_back() { back_ptr = decrement(back_ptr); }
  constexpr auto pop_front() { front_ptr = increment(front_ptr); }
  constexpr auto push_front(T val) { emplace<decrement>(front_ptr, val); }
  constexpr auto push_back(T val) { emplace<increment>(back_ptr, val); }

  constexpr auto size() const { return modulo(back_ptr - front_ptr); }
  constexpr auto operator[](size_t i) const { return arr[(front_ptr + i) % MaxSize]; }
  constexpr auto empty() const { return front_ptr == back_ptr; }
  constexpr auto nearly_full() const { return increment(back_ptr) == front_ptr; }

  constexpr auto front() const { return arr[front_ptr]; }
  constexpr auto back() const {
    if (size() <= 1) { return arr[front_ptr]; }
    return arr[decrement(back_ptr)];
  }

private:
  std::array<T, MaxSize> arr{};
  size_t front_ptr = 0;
  size_t back_ptr = 0;
  template <auto PointerFn>
  constexpr auto emplace(size_t& ptr, T val) {
    if constexpr (BoundsCheckOption == BoundsChecking::SilentReturn) {
      if (nearly_full()) { return; }
    }
    arr[ptr] = val;
    ptr = PointerFn(ptr);
  }
  constexpr static auto modulo(size_t val) { return (val + MaxSize) % MaxSize; }
  constexpr static auto increment(size_t ptr) { return modulo(ptr + 1); }
  constexpr static auto decrement(size_t ptr) { return modulo(ptr - 1); }
};

static_assert(deque<int, 10>{}.empty());
static_assert(deque<int, 10>{}.size() == 0);
static_assert([] {
  // looks like it works...
  auto dq = deque<int, 10>{};
  dq.push_back(1);
  dq.push_back(2);
  dq.push_back(3);
  dq.push_back(4);
  if (dq.back() != 4) { return false; }
  if (dq.size() != 4) { return false; }
  dq.pop_back();
  if (dq.size() != 3) { return false; }
  if (dq.back() != 3) { return false; }
  dq.pop_front();
  if (dq.size() != 2) { return false; }
  if (dq.front() != 2) { return false; }
  dq.pop_front();
  if (dq.size() != 1) { return false; }
  if (dq.front() != 3) { return false; }
  return true;
}());


// harder tests...
static_assert([] {
  auto dq = deque<int, 10, BoundsChecking::SilentReturn>{};
  dq.push_back(1);
  dq.push_back(2);
  dq.push_back(3);
  dq.push_back(4);
  dq.push_back(5);
  dq.push_back(6);
  dq.push_back(7);
  dq.push_back(8);
  dq.push_back(9);
  dq.push_back(10);
  // silent return for bounds checking
  if (dq.back() != 9) { return false; }
  return true;
}());

} // namespace pond

constexpr auto get_max_pool_out_size = [](auto N, auto K, auto Stride) {
  return (N / Stride) - K + 1;
};

template <typename T, size_t K>
constexpr auto max_pool_1d(const T* arr, size_t N)  {
  auto sz = get_max_pool_out_size(N, K, 1);
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

