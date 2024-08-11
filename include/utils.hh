#pragma once

#include <cstddef>
#include <functional>

template <class... Ts> struct match : Ts... {
  using Ts::operator()...;
};

template <class... Ts> match(Ts...) -> match<Ts...>;

inline void hash_combine(std::size_t &seed) {}

template <class T, class... Ts>
inline void hash_combine(std::size_t &seed, const T &v, Ts... rest) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  hash_combine(seed, rest...);
}

#define MAKE_HASHABLE(type, ...)                                               \
  namespace std {                                                              \
  template <> struct hash<type> {                                              \
    std::size_t operator()(const type &t) const {                              \
      std::size_t ret = 0;                                                     \
      hash_combine(ret, __VA_ARGS__);                                          \
      return ret;                                                              \
    }                                                                          \
  };                                                                           \
  }
