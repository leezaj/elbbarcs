#include "Game.h"
#include "GameOver.h"
#include "Playing.h"
#include <cassert>

namespace {
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

} // namespace

GameOver::GameOver(Playing &playing_state, const AssetPool &assets, ButtonMaker &button_maker) : 
  dialog_box_{assets.get(TextureType::DIALOG_BOX)}, 
  buttons_{
    Button{button_maker.make_text_button(Game::renderer(), "Restart", kRestartRect, [&playing_state] {
        playing_state.restart_game();
        Game::pop_until<Playing>();
      })}, 
    Button{button_maker.make_text_button(Game::renderer(), "Quit", kQuitRect, &Game::pop_until<MainMenu>)}
  }, 
  texts_ {
    Text{Game::renderer(), assets.get(FontType::MOULDY_CHEESE), kFontSizes[GAME_OVER], constants::kColorBrown},
    Text{Game::renderer(), assets.get(FontType::MOULDY_CHEESE), kFontSizes[RESULT], constants::kColorBrown},
    Text{Game::renderer(), assets.get(FontType::MOULDY_CHEESE), kFontSizes[PLAYER_SCORE], constants::kColorBeige},
    Text{Game::renderer(), assets.get(FontType::MOULDY_CHEESE), kFontSizes[PLAYER_DIFFERENCE], constants::kColorBeige},
    Text{Game::renderer(), assets.get(FontType::MOULDY_CHEESE), kFontSizes[OPP_SCORE], constants::kColorBeige},
    Text{Game::renderer(), assets.get(FontType::MOULDY_CHEESE), kFontSizes[OPP_DIFFERENCE], constants::kColorBeige},
    Text{Game::renderer(), assets.get(FontType::MOULDY_CHEESE), kFontSizes[HINTS_USED], constants::kColorBeige}
  },
  playing_state_{&playing_state}
{
  utility::log("Game over state initialized");
}

void GameOver::render_objects() const {
  SDL_RenderCopy(Game::renderer(), background_.get(), nullptr, nullptr);
  SDL_RenderCopy(Game::renderer(), dialog_box_, nullptr, &kDialogBoxRect);
  std::ranges::for_each(buttons_, [](const Button& button) static { button.render(Game::renderer()); });
  std::ranges::for_each(texts_, [](const Text &text) static {text.render(Game::renderer()); });
}

void GameOver::handle_event(const SDL_Event& event) {
  switch(event.type) {
  case SDL_MOUSEMOTION:
    hovered_button_ = Mouse::handle_hovering(buttons_);
    return;
  case SDL_MOUSEBUTTONUP:
    Mouse::click_hovered(hovered_button_);
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
    utility::log("Game over, player wins {} to {}", player_score, opponent_score);
    result = kWinText;
    hint_msg = hints_used ? kHintsUsed : kHintsUnusedWin;
  } else if(opponent_score > player_score){
    utility::log("Game over, opponent wins {} to {}", opponent_score, player_score);
    result = kLoseText;
    hint_msg = hints_used ? kHintsUsed : kHintsUnusedNoWin;
  } else {
    utility::log("Game over, draw from equal scores ({} and {})", player_score, opponent_score);
    result = kDrawText;
    hint_msg = hints_used ? kHintsUsed : kHintsUnusedNoWin;
  }
  texts_[GAME_OVER].update(Game::renderer(), "Game Over");
  texts_[RESULT].update(Game::renderer(), result);
  texts_[PLAYER_SCORE].update(Game::renderer(), "Your score: " + std::to_string(player_score));
  texts_[OPP_SCORE].update(Game::renderer(), "Computer score: " + std::to_string(opponent_score));
  texts_[PLAYER_DIFFERENCE].update(Game::renderer(), player_diff);
  texts_[OPP_DIFFERENCE].update(Game::renderer(), opponent_diff);
  texts_[HINTS_USED].update(Game::renderer(), hint_msg);
  for(size_t i = 0; i<NUM_OF_TEXTS; ++i) {
    texts_[i].center_text(kDialogBoxRect.x, kDialogBoxRect.w, kYPositions[i]);
  }
  Game::push_game_state(this);
  hovered_button_ = Mouse::handle_hovering(buttons_);
}
