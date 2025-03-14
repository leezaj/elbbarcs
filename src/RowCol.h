#ifndef ROWCOL_H
#define ROWCOL_H

#include "constants.h"
#include <SDL2/SDL_rect.h>
#include <cstdint>
struct Row_Col {
  std::int8_t row{};
  std::int8_t col{};
  friend bool operator==(Row_Col, Row_Col) = default;
};

[[nodiscard]] constexpr Row_Col to_row_col(SDL_Point point) {
  return {.row = static_cast<int8_t>(point.y / constants::kSquarePixelSize),
          .col = static_cast<int8_t>(point.x / constants::kSquarePixelSize)};
}

[[nodiscard]] constexpr SDL_Point to_point(Row_Col rc) {
  return {.x = rc.col * constants::kSquarePixelSize, .y = rc.row * constants::kSquarePixelSize};
}

template <> 
struct std::hash<Row_Col> {
  constexpr std::size_t operator()(Row_Col rc) const noexcept {
    static constexpr auto bit_size = std::numeric_limits<size_t>::digits;
    return (static_cast<size_t>(rc.row) << bit_size/2) | static_cast<size_t>(rc.col);
  }
};

#endif // ROWCOL_H
