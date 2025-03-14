#ifndef GAMEOVER_H
#define GAMEOVER_H

#include "AssetPool.h"
#include "Button.h"
#include "ButtonMaker.h"
#include "Text.h"
#include <array>

class Playing;

class GameOver final {
public:
  GameOver(Playing &playing_state, const AssetPool& assets, ButtonMaker &button_maker);

  void render_objects() const;

  void handle_event(const SDL_Event& event);

  void show(int player_score_, int opponent_score, int player_tile_sum, int opponent_tile_sum, bool hints_used_);
private:
  enum ButtonType : std::uint8_t { RESTART, QUIT, NUM_OF_BUTTONS };

  enum TextType : std::uint8_t { 
    GAME_OVER, 
    RESULT, 
    PLAYER_SCORE, 
    PLAYER_DIFFERENCE, 
    OPP_SCORE, 
    OPP_DIFFERENCE, 
    HINTS_USED, 
    NUM_OF_TEXTS 
  };

  static constexpr std::array<std::uint8_t, NUM_OF_TEXTS> kFontSizes{80, 60, 30, 20, 30, 20, 25};

  static constexpr std::array<std::uint16_t, NUM_OF_TEXTS> kYPositions{215, 300, 385, 425, 460, 490, 550};

  Texture background_;
  SDL_Texture* dialog_box_;
  std::array<Button, NUM_OF_BUTTONS> buttons_;
  std::array<Text, NUM_OF_TEXTS> texts_;
  const Button* hovered_button_{};
  Playing* playing_state_;
};

#endif //GAMEOVER_H
