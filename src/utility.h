#ifndef UTILITY_H
#define UTILITY_H

#include <SDL2/SDL_rect.h>
#include <array>
#include <bit>
#include <functional>
#include <random>
#include <ranges>

#include "constants.h"
#include <format>
#include <print>
#include <source_location>
#include <chrono>

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

namespace utility::impl {
class xoshiro256pp { // https://en.wikipedia.org/wiki/Xorshift#xoshiro256++
private:
  std::array<std::uint64_t, 4> state_;
public:
  using result_type = std::uint64_t;

  explicit xoshiro256pp(std::uint64_t seed = std::random_device{}()) {
    std::seed_seq seq{seed, seed+1, seed+2, seed+3};
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
} // namespace utility::impl

namespace Random {
  inline utility::impl::xoshiro256pp& engine() {
    static thread_local utility::impl::xoshiro256pp engine{};
    return engine;
  }
  [[nodiscard]] inline bool coin_flip() {
    return std::bernoulli_distribution{}(engine());
  }
} //namespace Random

namespace utility{

namespace impl{

template <typename T>
concept IsNotVoid = !std::is_void_v<T>;

template<typename MappingFunction, typename Range>
using MappedType = std::invoke_result_t<MappingFunction, std::ranges::range_value_t<Range>>;

constexpr std::string_view get_filename(std::string_view path) {
  size_t last_slash = path.find_last_of('/');
  if (last_slash == std::string_view::npos) {
    return path;
  }
  return path.substr(last_slash + 1);
}

struct format_string {
    consteval format_string(char const* str, std::source_location sloc = std::source_location::current()) : 
      str_{str} , sloc_{sloc}
    {}

    char const* str_;
    std::source_location sloc_;
};

} // namespace impl

template <typename... Args>
constexpr void log(impl::format_string fmt, Args&&... args) noexcept {
  if constexpr(constants::debug) {
    std::chrono::zoned_time current_time{std::chrono::current_zone(), std::chrono::system_clock::now()};
    std::println("[DEBUG] [{:%T}] {}:{} {}", 
      current_time,
      impl::get_filename(fmt.sloc_.file_name()),
      fmt.sloc_.line(),
      std::vformat(fmt.str_, std::make_format_args(args...)));
  }
}

template <template <typename...> typename Container = std::vector, typename R, typename F>
requires std::ranges::input_range<R> && std::regular_invocable<F, std::ranges::range_value_t<R>> &&
requires(std::ranges::range_value_t<R> value, F function){
  {std::invoke(function, value)} -> impl::IsNotVoid;
}
[[nodiscard]] constexpr auto map(R&& range, F&& transform_func) {
  using T = impl::MappedType<F,R>;
  return std::forward<R>(range) | std::views::transform(std::forward<F>(transform_func)) | std::ranges::to<Container<T>>();
}

} // namespace utility

#endif // UTILITY_H
