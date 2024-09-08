#ifndef GAMEOVER_H
#define GAMEOVER_H

#include "AssetPool.h"
#include "Background.h"
#include "Button.h"
#include "ButtonMaker.h"
#include "GameState.h"
#include "Text.h"
#include <array>

class Playing;

class GameOver final : public GameState {
public:
  GameOver(SDL_Renderer *renderer, Mouse &mouse, GameStateManager &manager, Playing &playing_state, 
           const AssetPool& assets, ButtonMaker &button_maker);

  void render_objects() const final;

  void handle_event(const SDL_Event& event) final;

  void show(int player_score_, int opponent_score, int player_tile_sum, int opponent_tile_sum, bool hints_used_);
private:
  Texture background_;
  Background dialog_box_;
  std::array<Button, 2> buttons_;
  std::array<Text, 7> texts_;
  GameStateManager* manager_;
  const Button* hovered_button_{};
  Playing* playing_state_;
};

#endif //GAMEOVER_H
