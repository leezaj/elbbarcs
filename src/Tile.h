#ifndef TILE_H
#define TILE_H

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <cstdint>

struct Tile final {
  SDL_Texture* texture;
  SDL_Rect rect;
  char letter;
  std::uint8_t value;

  Tile(SDL_Texture *tile_texture, const SDL_Rect &tile_rect, char tile_letter, std::uint8_t tile_value) : 
    texture{tile_texture},
    rect{tile_rect},
    letter{tile_letter},
    value{tile_value} 
  {}

  Tile() : Tile({}, {}, {}, {}) {}

  [[nodiscard]] SDL_Point point() const { return {.x = rect.x, .y = rect.y}; }

  void render(SDL_Renderer* renderer) const {
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
  };

  void move(SDL_Point point) noexcept {
    rect.x = point.x;
    rect.y = point.y;
  }

  void hover() const { SDL_SetTextureColorMod(texture, 240, 235, 225); }

  void unhover() const { SDL_SetTextureColorMod(texture, 255, 255, 255); }
};

#endif // TILE_H
