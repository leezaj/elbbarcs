#include "constants.h"
#include "Scoreboard.h"
#include <SDL_render.h>
#include <sstream>

constexpr SDL_Point kHumanScoreLocation{.x = 900, .y = 150};
constexpr SDL_Point kComputerScoreLocation{.x = 1040, .y = 150};
constexpr std::int32_t kScoreFontSize = 50;
constexpr SDL_Point kHumanIdLocation{.x = 910, .y = 110};
constexpr SDL_Point kComputerIdLocation{.x = 1015, .y = 110};
constexpr std::int32_t kIdFontSize = 25;

Scoreboard::Scoreboard(SDL_Renderer *renderer, const AssetPool& assets) :
  renderer_{renderer},
  human_score_(renderer, assets.get(FontType::LOWBALL), kScoreFontSize, constants::kFontColorBeige, "000", kHumanScoreLocation),
  computer_score_(renderer, assets.get(FontType::LOWBALL), kScoreFontSize, constants::kFontColorBeige, "000", kComputerScoreLocation),
  player_id_(renderer, assets.get(FontType::MOULDY_CHEESE), kIdFontSize, constants::kFontColorBrown, "You", kHumanIdLocation),
  computer_id_(renderer, assets.get(FontType::MOULDY_CHEESE), kIdFontSize, constants::kFontColorBrown, "Computer", kComputerIdLocation)
{}

void Scoreboard::add_score(Player player, std::int32_t amount) {
  auto [text, value, location] =
    player == Player::HUMAN ?
    std::tie(human_score_, human_score_val_,  kHumanScoreLocation) : 
    std::tie(computer_score_, computer_score_val_, kComputerScoreLocation);
  value += amount;
  std::ostringstream ss;
  ss << std::setfill('0') << std::setw(3) << value;
  text.update(renderer_, constants::kFontColorBeige, std::move(ss).str(), location);
}

void Scoreboard::render() const {
  human_score_.render(renderer_);
  computer_score_.render(renderer_);
  player_id_.render(renderer_);
  computer_id_.render(renderer_);
}

void Scoreboard::reset() {
  if(human_score_val_ != computer_score_val_ || human_score_val_ != 0) {
    add_score(Player::HUMAN, -human_score_val_);
    add_score(Player::COMPUTER, -computer_score_val_);
  }
}

int Scoreboard::get_score(Player player) const {
  return player == Player::HUMAN ? human_score_val_ : computer_score_val_;
}
