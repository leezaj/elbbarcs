#include "constants.h"
#include "WordOutliner.h"
#include <SDL2/SDL2_gfxPrimitives.h>

#include <cassert>

static constexpr std::int32_t kFontSize = 18;

static constexpr std::uint32_t kFontColorBlack = 0xFF000000;
static constexpr std::uint32_t kFontColorGreen = 0xFF008A00;
static constexpr std::uint32_t kFontColorRed = 0xFF0000BF;

WordOutliner::WordOutliner(SDL_Renderer* renderer, const AssetPool& assets):
  renderer_{renderer},
  score_(renderer, assets.get(FontType::LOWBALL), kFontSize, {})
{}

void WordOutliner::outline(SDL_Point begin, SDL_Point end, int score_amount) {
  color_ = kFontColorBlack;
  outline_common(begin, end, score_amount);
}

void WordOutliner::outline(SDL_Point begin, SDL_Point end, int score_amount, bool is_valid) {
  color_ = is_valid ? kFontColorGreen : kFontColorRed;
  outline_common(begin, end, score_amount);
}

void WordOutliner::outline_common(SDL_Point begin, SDL_Point end, int score_amount) {
  assert(begin.x == end.x or begin.y == end.y);
  rect_.x = begin.x;
  rect_.y = begin.y;
  if(begin.x == end.x) {
    rect_.w = constants::kSquarePixelSize;
    rect_.h = end.y + constants::kSquarePixelSize - rect_.y;
  } else {
    rect_.w = end.x + constants::kSquarePixelSize - rect_.x;
    rect_.h = constants::kSquarePixelSize;
  }
  begin.x+=2; // otherwise numbers starting with '1' will be hard to read
  score_.update(renderer_, {}, std::to_string(score_amount), begin);
  score_value_ = score_amount;
}
void WordOutliner::render() const {
  for(std::int16_t thickness = 0; thickness<2; ++thickness) {
    rectangleColor(renderer_, static_cast<std::int16_t>(rect_.x + rect_.w + (thickness)),
        static_cast<std::int16_t>(rect_.y + rect_.h + (thickness)),
        static_cast<std::int16_t>(rect_.x - (thickness)),
        static_cast<std::int16_t>(rect_.y - (thickness)), color_);
  }
  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
  score_.render(renderer_);
}
