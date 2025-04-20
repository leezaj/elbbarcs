#ifndef MOUSE_H
#define MOUSE_H

#include "Button.h"
#include "Tile.h"
#include "types.h"
#include "utility.h"
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_rect.h>

struct Mouse final {
public:
  static void set_default_cursor() {
    static Cursor default_cursor{SDL_CreateSystemCursor(SDL_SystemCursor::SDL_SYSTEM_CURSOR_ARROW)};
    SDL_SetCursor(default_cursor.get());
  }

  static void set_hand_cursor() {
    static Cursor hand_cursor{SDL_CreateSystemCursor(SDL_SystemCursor::SDL_SYSTEM_CURSOR_HAND)};
    SDL_SetCursor(hand_cursor.get());
  }

  static SDL_Point pos() {return pos_;}

  static void click_hovered(const Button* button) {
    if(button != nullptr) {
      button->click();
      button->unhover();
      set_default_cursor();
    }
  }

  template <std::ranges::range ... R> requires (Hoverable<std::ranges::range_value_t<R>> && ...)
  static constexpr auto handle_hovering(R&& ... things_to_check) {
    static_assert(sizeof...(things_to_check) != 0, "Must provide at least one range to check!");

    static constexpr std::array cursor_funcs{&set_default_cursor, &set_hand_cursor};

    // regular pointer if all ranges are same value type, variant of pointers otherwise (with no duplicate types)
    auto hovered = [&]{
      if constexpr(utility::all_same<std::ranges::range_value_t<R>...>) {
        return std::add_pointer_t<utility::first_t<std::ranges::range_value_t<R>...>>{};
      } else {
        return utility::deduplicate<std::variant, std::add_pointer_t<std::ranges::range_value_t<R>>...>{};
      }
    }();

    if(auto current_hover = ([&] {
      if(auto it = handle_hovering_impl(std::forward<R>(things_to_check)); it != nullptr) {
        hovered = it;
        return true;
      }
      return false;
    }() || ...); current_hover != hovering_status_) {
      hovering_status_ = current_hover;
      std::invoke(cursor_funcs[hovering_status_]);
    }

    return hovered;
  }

private:

  template <std::ranges::range R>
  static constexpr auto handle_hovering_impl(R&& range_to_check) {
    using T = std::ranges::range_value_t<R>;
    T* result = nullptr;
    for(auto&& selectable : std::forward<R>(range_to_check)) {
      if constexpr(requires(T obj){{obj.rect};}) {
        if (!contains(selectable.rect, pos_)) {
          selectable.unhover();
          continue;
        }
      } else if constexpr(requires(T obj){{obj.rectangle()};}) {
        if (!contains(selectable.rectangle(), pos_)) {
          selectable.unhover();
          continue;
        }
      }

      if constexpr (std::is_same_v<T, Button>) {
        if (selectable.is_enabled()) {
          selectable.hover();
          result = &selectable;
        }
      } else if constexpr (std::is_same_v<T, Tile>) {
        if (selectable.letter != constants::kTileGapChar) {
          selectable.hover();
          result = &selectable;
        }
      }
    }
    return result;
  }
  inline static constinit bool hovering_status_{false};
  inline static constinit SDL_Point pos_{};
  friend class Game;
};

#endif // MOUSE_H
