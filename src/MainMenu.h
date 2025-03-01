#ifndef MAINMENU_H
#define MAINMENU_H

#include "ButtonMaker.h"
#include "Playing.h"
#include <optional>
class MainMenu final {
public:
  MainMenu(const AssetPool &assets, ButtonMaker &button_maker);

  void render_objects() const;

  void handle_event(const SDL_Event& event);

private:
  enum Buttons : std::uint8_t {PLAY, CREDITS, NUM_BUTTONS};

  MainMenu(const AssetPool &assets,
           std::array<std::string_view, NUM_BUTTONS> texts,
           std::array<std::function<void()>, NUM_BUTTONS> callbacks);

  SDL_Texture* background_, *logo_;
  std::array<std::pair<Button, Texture>, NUM_BUTTONS> buttons_;
  const Button* hovered_button_{};
  std::optional<Playing> playing_state_;
};

#endif // MAINMENU_H
