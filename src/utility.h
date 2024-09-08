#ifndef UTILITY_H
#define UTILITY_H

#include <SDL2/SDL_rect.h>
#include <random>
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

#endif // UTILITY_H
