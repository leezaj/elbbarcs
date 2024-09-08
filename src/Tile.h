#ifndef TILE_H
#define TILE_H

#include "SelectableGameObject.h"
#include <cstdint>

class Tile final : public SelectableGameObject {
public:
  Tile(SDL_Texture *texture, const SDL_Rect &rect, char letter, std::uint8_t value) : 
    SelectableGameObject(texture, rect), 
    letter_{letter},
    value_{value} 
  {}

  Tile(): Tile({}, {}, {}, {}) {}

  [[nodiscard]] char letter() const noexcept { return letter_; }

  [[nodiscard]] auto value() const noexcept {return value_;}
private:
  char letter_;
  std::uint8_t value_;
};

#endif // TILE_H
