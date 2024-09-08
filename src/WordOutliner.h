#ifndef WORDOUTLINER_H
#define WORDOUTLINER_H

#include "AssetPool.h"
#include "Text.h"
#include <SDL2/SDL_render.h>
#include <cstdint>

class WordOutliner final {
public:
  explicit WordOutliner(SDL_Renderer* renderer, const AssetPool& assets);

  void outline(SDL_Point begin, SDL_Point end, int score_amount, bool is_valid);

  void outline(SDL_Point begin, SDL_Point end, int score_amount);

  [[nodiscard]] std::int32_t get_last_displayed() const {return score_value_;}

  void render() const;
private:
  void outline_common(SDL_Point begin, SDL_Point end, int score_amount);

  SDL_Renderer* renderer_;
  Text score_;
  SDL_Rect rect_{};
  std::uint32_t color_{};
  std::int32_t score_value_{};
};

#endif //WORDOUTLINER_H
