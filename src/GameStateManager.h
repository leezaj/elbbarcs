#include "types.h"
#include <SDL2/SDL_events.h>
#include <variant>
#include <vector>

template <GameState ... States>
class GameStateManager {
  using State = std::variant<std::add_pointer_t<States>...>;
public:
  void push(GameState auto* state) noexcept {
    states_.emplace_back(state);
    current_state_ = state;
  }

  void pop() noexcept {
    states_.pop_back();
    current_state_ = states_.back();
  }

  [[nodiscard]] size_t size() const noexcept {
    return states_.size();
  }

  void render_current_state() {
    std::visit([](GameState auto* state) static {
        state->render_objects();
    }, current_state_);
  }

  void current_state_handle_event(const SDL_Event& event) {
    std::visit([&](GameState auto* state) {
        state->handle_event(event);
    }, current_state_);
  }

  template <GameState T>
  void pop_until() {
    while(!states_.empty() && !std::holds_alternative<std::add_pointer_t<T>>(states_.back())) {
      pop();
    }
  }
private:
  State current_state_;
  std::vector<State> states_{};
};
