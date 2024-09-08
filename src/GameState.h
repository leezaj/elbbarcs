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
  /**
   * @brief The game state will render all of the game objects that it composes
   */
  virtual void render_objects() const = 0;

  /**
   * @brief The game state will handle all events it is able to handle
   */
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

  template <typename T> inline auto *handle_hovering(T&& things_to_check) {
    typename std::remove_reference_t<T>::value_type *it{nullptr};
    for (auto &selectable : things_to_check) {
      if (contains(selectable.rectangle(), mouse_pos())) {
        bool is_valid{};
        if constexpr(std::is_same_v<decltype(selectable), Button&>) {
          is_valid = selectable.is_enabled(); 
        } else if constexpr(std::is_same_v<decltype(selectable), Tile&>) {
          is_valid = selectable.letter() != constants::kTileGapChar;
        }
        if (is_valid) {
          it = &selectable;
          selectable.hover();
        }
      } else {
        selectable.unhover();
      }
    }
    it ? set_hand_cursor() : set_default_cursor();
    return it;
  }

  void click_hovered(auto* hovered) {
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
