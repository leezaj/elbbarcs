#ifndef BUTTON_H
#define BUTTON_H

#include "SelectableGameObject.h"
#include <cstdint>
#include <functional>
/**
 * @class Button
 * @brief A class that represents a button. A callback that takes no arguments
 * and returns nothing will be called when it is clicked.
 */
class Button final : public SelectableGameObject {
public:
  Button(SDL_Texture *texture, const SDL_Rect& rectangle, std::function<void()> callback) : 
    SelectableGameObject(texture, rectangle),
    callback_{std::move(callback)}
  {}

  /**
   * @brief Enables the button. The button will be clickable.
   */
  void enable() {
    static constexpr std::uint8_t full_opacity_alpha{255};
    SDL_SetTextureAlphaMod(texture(), full_opacity_alpha);
    enabled_ = true;
  }

  /**
   * @brief Disable the button. The button will not be clickable and will become transparent.
   */
  void disable() {
    static constexpr std::uint8_t semi_transparent_alpha{150};
    enabled_ = false;
    SDL_SetTextureAlphaMod(texture(), semi_transparent_alpha);
  }

  /**
   * @brief If enabled, invokes the callback.
   */
  void click() const noexcept {
    if (enabled_) {
      callback_();
    }
  }

  [[nodiscard]] bool is_enabled() const {return enabled_;}

  /**
   * @brief Change the callback of the button.
   */
  void set_click(std::function<void()> func) {callback_ = std::move(func);}
private:
  bool enabled_{true};
  std::function<void()> callback_;
};

#endif // BUTTON_H
