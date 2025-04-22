#ifndef UTILITY_H
#define UTILITY_H

#include "constants.h"
#include <SDL2/SDL_rect.h>
#include <array>
#include <bit>
#include <chrono>
#include <format>
#include <functional>
#include <print>
#include <random>
#include <ranges>
#include <source_location>

[[nodiscard]] constexpr SDL_Point operator-(SDL_Point p1, SDL_Point p2) {
  return SDL_Point{.x = p1.x - p2.x, .y = p1.y - p2.y};
}

[[nodiscard]] constexpr bool operator==(SDL_Point p1, SDL_Point p2){
  return p1.x == p2.x and p1.y == p2.y;
}

[[nodiscard]] constexpr bool contains(const SDL_Rect &rect, SDL_Point point) {
  return point.x >= rect.x and point.x < (rect.x + rect.w) and point.y >= rect.y and point.y < (rect.y + rect.h);
}

// TODO: Redundant once libstdc++ implements P2286R8
template <typename T>
struct std::formatter<std::vector<T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    // Actual formatting
    constexpr auto format(const std::vector<T>& vec, std::format_context& ctx) const {
        auto out = ctx.out();
        
        if (vec.empty()) {
            out = std::format_to(out, "[]");
            return out;
        }
        
        out = std::format_to(out, "[");
        
        for (size_t i = 0; i < vec.size() - 1; ++i) {
            out = std::format_to(out, "{}, ", vec[i]);
        }
        
        out = std::format_to(out, "{}]", vec.back());
        
        return out;
    }
};

namespace utility{

namespace impl{

class xoshiro256pp { // https://en.wikipedia.org/wiki/Xorshift#xoshiro256++
private:
  std::array<std::uint64_t, 4> state_;
public:
  using result_type = std::uint64_t;

  constexpr xoshiro256pp() {
    std::random_device rd{};
    std::seed_seq seq{rd(), rd(), rd(), rd()};
    seq.generate(state_.begin(), state_.end());
  }

  constexpr std::uint64_t operator()() {
    const std::uint64_t result = std::rotl(state_[0] + state_[3], 23) + state_[0];
    const std::uint64_t t = state_[1] << 17U;

    state_[2] ^= state_[0];
    state_[3] ^= state_[1];
    state_[1] ^= state_[2];
    state_[0] ^= state_[3];

    state_[2] ^= t;
    state_[3] = std::rotl(state_[3], 45);

    return result;
  }

    static constexpr uint64_t min() { return 0; }
    static constexpr uint64_t max() { return std::numeric_limits<std::uint64_t>::max(); }
};
static_assert(std::uniform_random_bit_generator<xoshiro256pp>);

constexpr std::string_view get_filename(std::string_view path) {
  if (size_t last_slash = path.find_last_of('/'); last_slash != std::string_view::npos) {
    return path.substr(last_slash + 1);
  }
  return path;
}

struct format_string {
  consteval format_string(char const* str, std::source_location sloc = std::source_location::current()) : 
    str_{str} , sloc_{sloc}
  {}

  char const* str_;
  std::source_location sloc_;
};

// example: variant<int, char, char, double, int, int, int, double> -> variant<int, char, double>
template <template <typename ...> typename T, typename ... Ts>
struct deduplicate {
  using type = T<Ts...>;

  template<typename U>
  using result_t = std::conditional_t<
    std::disjunction_v<std::is_same<Ts, U>...>, 
    deduplicate<T, Ts...>, 
    deduplicate<T, Ts..., U>
  >;

  template<typename U>
  consteval result_t<U> operator+(std::type_identity<U>);
};

} // namespace impl

template <typename ... Ts>
using first_t = std::tuple_element_t<0, std::tuple<Ts...>>;

template<template <typename...> typename T, typename... Ts> requires (sizeof...(Ts) != 0)
using deduplicate = decltype((impl::deduplicate<T, first_t<Ts...>>() + ... + std::type_identity<Ts>()))::type;

template<typename ... Ts>
concept all_same = (std::same_as<first_t<Ts...>, Ts> && ...);

template <typename... Args>
constexpr void log(impl::format_string fmt, const Args&... args) noexcept {
  if constexpr(constants::debug) {
    std::chrono::zoned_time current_time{std::chrono::current_zone(), std::chrono::system_clock::now()};
    std::println("[DEBUG] [{:%T}] {}:{} {}", 
      current_time,
      impl::get_filename(fmt.sloc_.file_name()),
      fmt.sloc_.line(),
      std::vformat(fmt.str_, std::make_format_args(args...))
    );
  }
}

template <template <typename...> typename Container = std::vector, std::ranges::input_range R, typename F>
[[nodiscard]] constexpr auto map(R&& range, F&& transform) noexcept {
  using result_type = std::remove_cvref_t<std::invoke_result_t<F, std::ranges::range_value_t<R>>>;
  static_assert(not std::is_void_v<result_type>, "Mapping function F must not return void!");

  return std::forward<R>(range) | std::views::transform(std::forward<F>(transform)) | std::ranges::to<Container<result_type>>();
}

// Specialization sentinel for map<array>: we can't use `std::array` as a template due to its NTTP
struct to_array {};

template<typename Sentinel, std::ranges::input_range R, typename F> 
requires std::same_as<Sentinel, to_array> && std::regular_invocable<F, std::ranges::range_value_t<R>>
[[nodiscard]] constexpr auto map(const R& range, F&& transform_func) noexcept {
  using result_type = std::remove_cvref_t<std::invoke_result_t<F, std::ranges::range_value_t<R>>>;
  static_assert(not std::is_void_v<result_type>, "Mapping function F must not return void!");

  return [&transform_func, it = range.begin()]
    <size_t... Is>( std::index_sequence<Is...>) mutable {
      return std::array<result_type, sizeof...(Is)>{ ((void)Is, std::invoke(std::forward<F>(transform_func), *(it++)))...};
  }(std::make_index_sequence<range.size()>{});
}

template<size_t N, typename F> requires std::regular_invocable<F>
[[nodiscard]] constexpr auto generate_array(F&& generator) noexcept {
  using result_type = std::remove_cvref_t<std::invoke_result_t<F>>;
  static_assert(not std::is_void_v<result_type>, "Generating function F must not return void!");
  return [func = std::forward<F>(generator)]<typename Self, size_t... Is> (this Self&&, std::index_sequence<Is...>) 
    { 
      return std::array<result_type, N>{ ((void)Is, std::invoke(std::forward<F>(func)))... }; 
    } (std::make_index_sequence<N>{});
}

namespace random {

inline utility::impl::xoshiro256pp& engine() {
  static thread_local utility::impl::xoshiro256pp engine{};
  return engine;
}
[[nodiscard]] inline constexpr bool coin_flip() {
  return static_cast<bool>(engine()() & 1);
}

} //namespace random

} // namespace utility

#endif // UTILITY_H
