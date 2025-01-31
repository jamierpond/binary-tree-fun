#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

// only supports power of two T sizes
template <typename T>
using uint_t = std::conditional_t<sizeof(T) == 1, std::uint8_t,
               std::conditional_t<sizeof(T) == 2, std::uint16_t,
               std::conditional_t<sizeof(T) == 4, std::uint32_t,
               std::conditional_t<sizeof(T) == 8, std::uint64_t,
               std::conditional_t<sizeof(T) == 16, __uint128_t, // wtf this is wild
               void>>>>>;

template <typename T>
constexpr auto to_byte_array(T in) {
  auto val = std::bit_cast<uint_t<T>>(in);
  constexpr auto size = sizeof(T);
  auto a = std::array<std::byte, size>{};
  for (auto i = 0; i < size; i++) {
    a[i] = static_cast<std::byte>((val >> (i * 8)) & 0xFF);
  }
  return a;
}

template <typename T>
constexpr auto from_byte_array(const std::array<std::byte, sizeof(T)>& arr) {
  uint_t<T> val = 0;
  for (auto i = 0; i < sizeof(T); i++) {
    val |= static_cast<uint_t<T>>(arr[i]) << (i * 8);
  }
  return std::bit_cast<T>(val);
}

typedef std::array<std::byte, 4> int_bytes;

static_assert(to_byte_array(1) == int_bytes{std::byte{1}, std::byte{0}, std::byte{0}, std::byte{0}});
static_assert(from_byte_array<int>(int_bytes{std::byte{1}, std::byte{0}, std::byte{0}, std::byte{0}}) == 1);
static_assert(to_byte_array(0x12345678) == int_bytes{std::byte{0x78}, std::byte{0x56}, std::byte{0x34}, std::byte{0x12}});

template <typename T>
constexpr auto test(T val) {
  return from_byte_array<T>(to_byte_array(val)) == val;
}

static_assert(test(1));
static_assert(test(0x12345678));
static_assert(test(0x12345678));
static_assert(test<float>(45.0f));
static_assert(test<int16_t>(-45));

// wow kinda wild this works
struct SixteenByteStruct {
  // 16 bytes!
  int x;
  float y;
  double z;
  constexpr bool operator==(const SixteenByteStruct& other) const {
    return x == other.x && y == other.y && z == other.z;
  }
};
static_assert(sizeof(SixteenByteStruct) == 16);

// static_assert(sizeof(Foo) == 8);
// static_assert(test<TwelveByteStruct>({1, 2.9f, 3}));
static_assert(std::is_same_v<uint_t<SixteenByteStruct>, __uint128_t>);
static_assert(std::is_same_v<decltype(to_byte_array(SixteenByteStruct{1, 2.9f, 3.0})), std::array<std::byte, 16>>);
static_assert(test<SixteenByteStruct>({1, 2.9f, 3.0}));
