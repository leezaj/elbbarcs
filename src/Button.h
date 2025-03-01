#ifndef BUTTON_H
#define BUTTON_H

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <cstdint>
#include <functional>

/**
 * @class Button
 * @brief A class that represents a button. A callback that takes no arguments
 * and returns nothing will be called when it is clicked.
 */
class Button final {
public:
  Button(SDL_Texture *texture, const SDL_Rect& rect, std::function<void()> callback) : 
    texture_{texture},
    rect_{rect},
    callback_{std::move(callback)}
  {}

  void enable() {
    static constexpr std::uint8_t full_opacity_alpha{255};
    SDL_SetTextureAlphaMod(texture_, full_opacity_alpha);
    enabled_ = true;
  }

  void disable() {
    static constexpr std::uint8_t semi_transparent_alpha{150};
    SDL_SetTextureAlphaMod(texture_, semi_transparent_alpha);
    enabled_ = false;
  }

  [[nodiscard]] bool is_enabled() const {return enabled_;}

  void click() const noexcept {
    if (enabled_) {
      callback_();
    }
  }

  void hover() const { SDL_SetTextureColorMod(texture_, 240, 235, 225); }

  void unhover() const { SDL_SetTextureColorMod(texture_, 255, 255, 255); }

  void render(SDL_Renderer* renderer) const {
    SDL_RenderCopy(renderer, texture_, nullptr, &rect_);
  };

  [[nodiscard]] const auto& rectangle() const { return rect_; }

  void set_click(std::function<void()> func) {callback_ = std::move(func);}

private:
  SDL_Texture* texture_;
  SDL_Rect rect_;
  std::function<void()> callback_;
  bool enabled_{true};
};

#endif // BUTTON_H
