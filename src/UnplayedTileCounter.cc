#include "UnplayedTileCounter.h"
#include "constants.h"
#include <map>
#include <ranges>
#include <unordered_map>
#include <unordered_set>

namespace {

constexpr auto kTileW = static_cast<int>(constants::kTileWidth / 1.4);
constexpr auto kTileH = static_cast<int>(constants::kTileHeight / 1.4);
constexpr auto kWidthGap = 15;
constexpr auto kHeightGap = 15;
constexpr auto kRows = 5;
constexpr auto kTilesPerRow = constants::kNumOfTiles / kRows;
constexpr auto kCounterFontSize = 22;
constexpr SDL_Rect kRect{
    .x = 850,
    .y = 300,
    .w = (kTileW + kWidthGap) * kTilesPerRow - kWidthGap,
    .h = (kTileH + kHeightGap) * (kRows + 1) - kHeightGap};


// the purpose of this is to order the blank tile after all letters no matter what its value actually is
struct CharComparator {
  static bool operator()(char first, char second) {
    if (first == constants::kTileBlankChar) {
      return false;
    }
    if (second == constants::kTileBlankChar) {
      return true;
    }
    return first < second;
  }
};

} // namespace

UnplayedTileCounter::UnplayedTileCounter(SDL_Renderer *renderer, const AssetPool &assets, std::span<const Tile> all_tiles) : 
  all_tiles_{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, kRect.w, kRect.h)},
  blacked_out_tiles_{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, kRect.w, kRect.h)}
{
  constexpr auto kBackgroundAlpha = 175U;
  SDL_SetTextureBlendMode(all_tiles_.get(), SDL_BLENDMODE_BLEND);
  SDL_SetTextureBlendMode(blacked_out_tiles_.get(), SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, blacked_out_tiles_.get());
  SDL_RenderClear(renderer);
  SDL_SetRenderTarget(renderer, all_tiles_.get());
  SDL_SetRenderDrawColor(renderer, constants::kFontColorBrown.r, constants::kFontColorBrown.g, constants::kFontColorBrown.b, kBackgroundAlpha);
  SDL_RenderClear(renderer);
  SDL_SetRenderTarget(renderer, nullptr);
  SDL_SetRenderDrawColor(renderer, 0, 0 ,0 ,0);
  std::map<char, SDL_Texture*, CharComparator> letter_to_texture{};
  original_freqs_.reserve(constants::kNumOfTiles);
  counters_.reserve(constants::kNumOfTiles);
  for(const Tile& tile: all_tiles) {
    letter_to_texture.try_emplace(tile.letter(), tile.texture());
    ++original_freqs_[tile.letter()];
  }
  auto *const counter_font = assets.get(FontType::LOWBALL);
  SDL_Rect current_counter_rect{.x = 0, .y = 0, .w = kTileW, .h = kTileH};
  SDL_SetRenderTarget(renderer, all_tiles_.get());
  for(int idx = 0; auto [letter, texture] : letter_to_texture) {
    auto frequency = original_freqs_.at(letter);
    current_counter_rect.x = (kTileW + kWidthGap) * (idx % kTilesPerRow);
    current_counter_rect.y = (kTileH + kHeightGap) * (idx / kTilesPerRow);
    counters_.try_emplace(letter, renderer, counter_font, kCounterFontSize, SDL_Color{.r = 0, .g = 0, .b = 0, .a = 0},
                          std::to_string(frequency),
                          // we add +2 to the x of the number that it's not
                          // right on the border of the tile (easier to read)
                          SDL_Point{.x = current_counter_rect.x + kRect.x + 2,
                                    .y = current_counter_rect.y + kRect.y});
    SDL_RenderCopy(renderer, texture, nullptr, &current_counter_rect);
    ++idx;
  }
  SDL_SetRenderTarget(renderer, nullptr);
  current_freqs_ = original_freqs_;
}

void UnplayedTileCounter::update_count(SDL_Renderer *renderer, std::span<const Tile> played_tiles) {
  static constexpr SDL_Color shadow{.r = 0, .g = 0, .b = 0, .a = 100};
  std::unordered_set<char> to_update{};
  auto total = played_tiles.size();
  to_update.reserve(total - already_checked_idx_);
  for (; already_checked_idx_ < total; ++already_checked_idx_) {
    char letter = played_tiles[already_checked_idx_].letter();
    --current_freqs_[letter];
    to_update.emplace(letter);
  }
  for (char letter : to_update) {
    int new_freq = current_freqs_.at(letter);
    Text &counter = counters_.at(letter);
    if (new_freq == 0) {
      // -2 to the x because we added 2 earlier
      SDL_Rect black_shadow{.x = counter.x() - kRect.x - 2, .y = counter.y() - kRect.y, .w = kTileW, .h = kTileH};
      SDL_SetRenderTarget(renderer, blacked_out_tiles_.get());
      SDL_SetRenderDrawColor(renderer, shadow.r, shadow.g, shadow.b, shadow.a);
      SDL_RenderFillRect(renderer, &black_shadow);
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_SetRenderTarget(renderer, nullptr);
    }
    counter.update(renderer, std::to_string(new_freq));
  }
}

void UnplayedTileCounter::reset(SDL_Renderer* renderer) {
  current_freqs_ = original_freqs_;
  for(auto [letter, frequency]: current_freqs_) {
    counters_.at(letter).update(renderer, std::to_string(frequency));
  }
  SDL_SetRenderTarget(renderer, blacked_out_tiles_.get());
  SDL_RenderClear(renderer);
  SDL_SetRenderTarget(renderer, nullptr);
  already_checked_idx_ = 0;
}

void UnplayedTileCounter::render(SDL_Renderer* renderer) const {
  SDL_RenderCopy(renderer, all_tiles_.get(), nullptr, &kRect);
  std::ranges::for_each( counters_ | std::views::values, [=](const Text &text) { text.render(renderer); });
  SDL_RenderCopy(renderer, blacked_out_tiles_.get(), nullptr, &kRect);
}
