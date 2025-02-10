#ifndef UTILITY_H
#define UTILITY_H

#include <SDL2/SDL_rect.h>
#include <random>
#include <algorithm>
#include <iterator>
namespace Random {
  inline std::mt19937 engine{std::random_device{}()};
  [[nodiscard]] inline bool coin_flip() {
    return std::bernoulli_distribution{}(engine);
  }
} //namespace Random

[[nodiscard]] inline SDL_Point operator-(SDL_Point p1, SDL_Point p2) {
  return SDL_Point{.x = p1.x - p2.x, .y = p1.y - p2.y};
}

[[nodiscard]] inline constexpr bool operator==(SDL_Point p1, SDL_Point p2){
  return p1.x == p2.x and p1.y == p2.y;
}

[[nodiscard]] inline constexpr bool contains(const SDL_Rect &rect, SDL_Point point) {
  return point.x >= rect.x and point.x < (rect.x + rect.w) and point.y >= rect.y and point.y < (rect.y + rect.h);
}


namespace utility{
template <typename T>
concept IsNotVoid = !std::is_void_v<T>;

template<typename MappingFunction, typename Range>
using MappedType = std::invoke_result_t<MappingFunction, std::ranges::range_value_t<Range>>;

template <typename Container>
concept BackInsertable = requires(Container container) {
  {container.push_back(std::declval<typename Container::value_type>())} -> std::same_as<void>;
  {std::back_inserter(container)} -> std::output_iterator<typename Container::value_type>;
};

template <typename Container>
concept Insertable = requires(Container container) {
  {std::inserter(container, container.end())} -> std::output_iterator<typename Container::value_type>;
};

template <typename Container>
concept Reservable = requires(Container container) {
  {container.reserve(0)} -> std::same_as<void>;
};

template <template <typename...> typename Container = std::vector, typename R, typename F>
requires std::ranges::sized_range<R> && requires(std::ranges::range_value_t<R> value, F function){
  requires std::invocable<F, std::ranges::range_value_t<R>>;
  {function(value)} -> IsNotVoid;
  typename Container<MappedType<F,R>>;
} && (BackInsertable<Container<MappedType<F,R>>> || Insertable<Container<MappedType<F,R>>>)

[[nodiscard]] constexpr Container<MappedType<F,R>> map(R&& range, F&& transform_func) {
  Container<MappedType<F,R>> result{};
  if constexpr (Reservable<Container<MappedType<F,R>>>) {
    result.reserve(std::ranges::size(range));
  }
  if constexpr(BackInsertable<Container<MappedType<F,R>>>) {
    std::ranges::transform(range, std::back_inserter(result), transform_func);
  } else if constexpr(Insertable<Container<MappedType<F,R>>>){
    std::ranges::transform(range, std::inserter(result, result.end()), transform_func);
  } else {
    static_assert(false, "The container you provide must support `push_back` or `insert`.");
  }
  return result;
}
} // namespace utility

#endif // UTILITY_H
