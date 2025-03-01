#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Button.h"
#include "Mouse.h"
#include "Tile.h"
#include "constants.h"
#include "utility.h"
#include <SDL2/SDL_events.h>

/**
 * @class GameState
 * @brief A game state represents a certain state the game is in.
 *
 */
class GameState {
public:

  virtual void render_objects() const = 0;

  virtual void handle_event(const SDL_Event &event) = 0;

  virtual ~GameState() = default;
protected:
  GameState(SDL_Renderer *rend, Mouse& mouse) : 
    renderer_{rend}, mouse_{&mouse}
  {}

  GameState(const GameState &) = default;
  GameState(GameState &&) = default;
  GameState &operator=(const GameState &) = default;
  GameState &operator=(GameState &&) = default;

  [[nodiscard]] SDL_Point mouse_pos() const { return mouse_->pos(); }

  void set_mouse_pos(SDL_Point mouse_pos) {mouse_->set_pos(mouse_pos);}

  [[nodiscard]] SDL_Renderer *renderer() const { return renderer_; }

  void set_hand_cursor() const {mouse_->set_hand_cursor();}

  void set_default_cursor() const {mouse_->set_default_cursor();}

  template <std::ranges::range R> requires std::derived_from<std::ranges::range_value_t<R>, SelectableGameObject>
  auto *handle_hovering(R& things_to_check) {

    using T = std::ranges::range_value_t<R>;
    static bool last_hover = false;

    static constexpr auto cursor_funcs = std::array{
      &GameState::set_default_cursor,
      &GameState::set_hand_cursor
    };

    auto it = std::ranges::find_if(things_to_check, [this](T& selectable) {
        if (!contains(selectable.rectangle(), mouse_pos())) {
            selectable.unhover();
            return false;
        }

        if constexpr (std::is_same_v<T, Button>) {
            if (!selectable.is_enabled()) {
              return false;
            }
        } else if constexpr (std::is_same_v<T, Tile>) {
            if (selectable.letter() == constants::kTileGapChar) {
              return false;
            }
        }

        selectable.hover();
        return true;
    });

    bool current_hover = (it != things_to_check.end());

    if (current_hover != last_hover) {
      std::invoke(cursor_funcs[current_hover], this);
      last_hover = current_hover;
    }

    return (current_hover ? &*it : nullptr);
}

  void click_hovered(const Button* hovered) {
    if(hovered == nullptr) {
      return;
    }
    hovered->unhover();
    hovered->click();
    set_default_cursor();
  }

private:
  SDL_Renderer *renderer_;
  Mouse* mouse_;
};

#endif // GAMESTATE_H
