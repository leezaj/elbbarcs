#include "GameOver.h"
#include "Playing.h"
#include <cassert>

namespace {
enum Buttons : std::uint8_t { RESTART, QUIT };
enum Texts : std::uint8_t {
  GAME_OVER,
  RESULT,
  PLAYER_SCORE,
  PLAYER_DIFFERENCE,
  OPP_SCORE,
  OPP_DIFFERENCE,
  HINTS_USED,
  NUM_TEXTS
};

constexpr SDL_Rect kDialogBoxRect{.x = 71, .y = 71, .w = 1050, .h = 725};

constexpr SDL_Rect kRestartRect{
  .x = kDialogBoxRect.x + 300,
  .y = kDialogBoxRect.y + kDialogBoxRect.h - 185,
  .w = 180,
  .h = 96
};
constexpr SDL_Rect kQuitRect{
  .x = kDialogBoxRect.w - kDialogBoxRect.x - 350,
  .y = kDialogBoxRect.y + kDialogBoxRect.h - 185,
  .w = 180,
  .h = 96
};

constexpr std::string_view
    kWinText = "You won!",
    kLoseText = "You lost!", 
    kDrawText = "Draw!",
    kHintsUsed = "You used hints; can you win next time without using any?",
    kHintsUnusedWin = "Congrats, you managed to win without using hints!!! You're awesome!",
    kHintsUnusedNoWin = "Well done playing without hints!";

constexpr std::array<int, NUM_TEXTS> kYPositions{215, 300, 385, 425, 460, 490, 550};

constexpr std::array<int, NUM_TEXTS> kFontSizes{80, 60, 30, 20, 30, 20, 25};
} // namespace

GameOver::GameOver(SDL_Renderer *renderer, Mouse &mouse, GameStateManager &manager, Playing &playing_state,
                   const AssetPool &assets, ButtonMaker &button_maker) : 
  GameState{renderer, mouse}, 
  dialog_box_{assets.get(TextureType::DIALOG_BOX), kDialogBoxRect}, 
  buttons_{
    Button{button_maker.make_text_button(renderer, "Restart", kRestartRect, [&playing_state, &manager] {
        playing_state.restart_game();
        manager.pop();
      })}, 
    Button{button_maker.make_text_button(renderer, "Quit", kQuitRect, [&manager] {
        while(manager.size()!=1){
          manager.pop();
        }
      })}
  }, 
  texts_ {
    Text{renderer, assets.get(FontType::MOULDY_CHEESE), kFontSizes[GAME_OVER], constants::kFontColorBrown},
    Text{renderer, assets.get(FontType::MOULDY_CHEESE), kFontSizes[RESULT], constants::kFontColorBrown},
    Text{renderer, assets.get(FontType::MOULDY_CHEESE), kFontSizes[PLAYER_SCORE], constants::kFontColorBeige},
    Text{renderer, assets.get(FontType::MOULDY_CHEESE), kFontSizes[PLAYER_DIFFERENCE], constants::kFontColorBeige},
    Text{renderer, assets.get(FontType::MOULDY_CHEESE), kFontSizes[OPP_SCORE], constants::kFontColorBeige},
    Text{renderer, assets.get(FontType::MOULDY_CHEESE), kFontSizes[OPP_DIFFERENCE], constants::kFontColorBeige},
    Text{renderer, assets.get(FontType::MOULDY_CHEESE), kFontSizes[HINTS_USED], constants::kFontColorBeige}
  },
  manager_{&manager},
  playing_state_{&playing_state}
{}

void GameOver::render_objects() const {
  SDL_RenderCopy(renderer(), background_.get(), nullptr, nullptr);
  dialog_box_.render(renderer());
  std::ranges::for_each(buttons_, [this](const Button& button) { button.render(renderer()); });
  std::ranges::for_each(texts_, [this](const Text &text) {text.render(renderer()); });
}

void GameOver::handle_event(const SDL_Event& event) {
  switch(event.type) {
  case SDL_MOUSEMOTION:
    set_mouse_pos({.x = event.motion.x, .y=event.motion.y});
    hovered_button_ = handle_hovering(buttons_);
    return;
  case SDL_MOUSEBUTTONUP:
    click_hovered(hovered_button_);
    return;
  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_ESCAPE) {
      buttons_[QUIT].click();
    } else if (event.key.keysym.sym == SDLK_RETURN) {
      buttons_[RESTART].click();
    }
    return;
  default:
    return;
  }
}

void GameOver::show(int player_score, int opponent_score, int player_tile_sum, int opponent_tile_sum, bool hints_used) {
  background_ = playing_state_->get_snapshot();
  static constexpr auto kDarkDim = constants::kDimRGB - 50;
  SDL_SetTextureColorMod(background_.get(), kDarkDim, kDarkDim, kDarkDim);
  bool player_out = opponent_tile_sum != 0 and player_tile_sum == 0;
  bool opoonent_out = opponent_tile_sum == 0 and player_tile_sum != 0;
  std::string_view result, hint_msg;
  std::string player_diff, opponent_diff;
  if(opoonent_out) {
    opponent_score += (player_tile_sum *= 2);
    opponent_diff = "(includes " + std::to_string(player_tile_sum) + " points from your unplayed tiles)";
  } else if (player_out) {
    player_score += (opponent_tile_sum *= 2);
    player_diff = "(includes " + std::to_string(opponent_tile_sum) + " points from the opponent's unplayed tiles)";
  } else { // 'Six-Zero Rule'
    opponent_diff = "(subtracted " + std::to_string(opponent_tile_sum) + " points from unplayed tiles)";
    player_diff = "(subtracted " + std::to_string(player_tile_sum) + " points from unplayed tiles)";
    player_score -= player_tile_sum;
    opponent_score -= opponent_tile_sum;
  }
  if(player_score > opponent_score) {
    result = kWinText;
    hint_msg = hints_used ? kHintsUsed : kHintsUnusedWin;
  } else if(opponent_score > player_score){
    result = kLoseText;
    hint_msg = hints_used ? kHintsUsed : kHintsUnusedNoWin;
  } else {
    result = kDrawText;
    hint_msg = hints_used ? kHintsUsed : kHintsUnusedNoWin;
  }
  texts_[GAME_OVER].update(renderer(), "Game Over");
  texts_[RESULT].update(renderer(), result);
  texts_[PLAYER_SCORE].update(renderer(), "Your score: " + std::to_string(player_score));
  texts_[OPP_SCORE].update(renderer(), "Computer score: " + std::to_string(opponent_score));
  texts_[PLAYER_DIFFERENCE].update(renderer(), player_diff);
  texts_[OPP_DIFFERENCE].update(renderer(), opponent_diff);
  texts_[HINTS_USED].update(renderer(), hint_msg);
  for(size_t i = 0; i<NUM_TEXTS; ++i) {
    texts_[i].center_text(kDialogBoxRect.x, kDialogBoxRect.w, kYPositions[i]);
  }
  manager_->push(this);
  hovered_button_ = handle_hovering(buttons_);
}
