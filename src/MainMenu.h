#ifndef MAINMENU_H
#define MAINMENU_H

#include "ButtonMaker.h"
#include "GameState.h"
#include "Playing.h"
#include <optional>
class MainMenu final : public GameState {
public:
  MainMenu(SDL_Renderer *rend, Mouse& mouse, const AssetPool &assets, GameStateManager& manager, ButtonMaker& button_maker);
  void render_objects() const final;
  void handle_event(const SDL_Event& event) final;
private:
  Background logo_;
  SDL_Texture* background_;
  std::vector<Texture> button_textures_;
  std::vector<Button> buttons_; 
  const Button* hovered_button_{};
  std::optional<Playing> playing_state_;
};

#endif // MAINMENU_H
