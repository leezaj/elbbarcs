#include "constants.h"
#include <SDL2/SDL_rect.h>
#include <boost/functional/hash.hpp>
#include <cstdint>
struct Row_Col {
  std::int8_t row{};
  std::int8_t col{};
  friend bool operator==(Row_Col, Row_Col) = default;
};

[[nodiscard]] inline Row_Col to_row_col(SDL_Point point) {
  return {.row = static_cast<int8_t>(point.y / constants::kSquarePixelSize),
          .col = static_cast<int8_t>(point.x / constants::kSquarePixelSize)};
}

[[nodiscard]] inline SDL_Point to_point(Row_Col rc) {
  return {.x = rc.col * constants::kSquarePixelSize, .y = rc.row * constants::kSquarePixelSize};
}

template <> 
struct std::hash<Row_Col> {
  std::size_t operator()(const Row_Col &rc) const noexcept {
    std::size_t ret{};
    boost::hash_combine(ret, rc.row);
    boost::hash_combine(ret, rc.col);
    return ret;
  }
};
