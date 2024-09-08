#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "AssetPool.h"
#include "Text.h"

class Scoreboard final {
public:
  enum class Player : std::uint8_t { HUMAN, COMPUTER };

  Scoreboard(SDL_Renderer *renderer, const AssetPool& assets);

  void add_score(Player player, std::int32_t amount);

  [[nodiscard]] std::int32_t get_score(Player player) const;

  void render() const;

  void reset();
private:
  SDL_Renderer* renderer_;
  std::int32_t human_score_val_{}, computer_score_val_{};
  Text human_score_, computer_score_, player_id_, computer_id_;
};

#endif // SCOREBOARD_H
