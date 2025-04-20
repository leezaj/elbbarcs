#include "UnplayedTileCounter.h"
#include "constants.h"
#include "utility.h"
#include <ranges>

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
    .w = ((kTileW + kWidthGap) * kTilesPerRow) - kWidthGap,
    .h = ((kTileH + kHeightGap) * (kRows + 1)) - kHeightGap};



// Requires that the tiles are sorted by their letters
constexpr std::array<SDL_Texture*, constants::kNumOfTiles> unique_textures(std::span<const Tile> tiles) {
  std::array<SDL_Texture*, constants::kNumOfTiles> result;
  char previously_seen = std::numeric_limits<char>::max();
  for(size_t curr_idx = 0; const Tile& tile: tiles) {
    if(auto letter = tile.letter; letter != previously_seen) {
      result[curr_idx++] = tile.texture;
      previously_seen = letter;
    }
  }
  return result;
}

} // namespace

UnplayedTileCounter::UnplayedTileCounter(SDL_Renderer *renderer, const AssetPool &assets, std::span<const Tile> all_tiles) : 
  UnplayedTileCounter{renderer, assets.get(FontType::LOWBALL), unique_textures(all_tiles)}
{}

UnplayedTileCounter::UnplayedTileCounter(SDL_Renderer* renderer, TTF_Font* counter_font, std::array<SDL_Texture*, constants::kNumOfTiles> unique_textures) :
  all_tiles_{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, kRect.w, kRect.h)},
  unavailable_shadows_{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, kRect.w, kRect.h)},
  texts_{
    utility::generate_array<unique_textures.size()>( [renderer, counter_font, index = 0] mutable -> Text {
        const int tile_x = (kTileW + kWidthGap) * (index % kTilesPerRow);
        const int tile_y = (kTileH + kHeightGap) * (index / kTilesPerRow);
        return Text{
          renderer, counter_font, kCounterFontSize, SDL_Color{.r = 0, .g = 0, .b = 0, .a = 0}, 
          std::to_string(constants::tile_info[static_cast<size_t>(index++)].frequency),
          SDL_Point{.x = tile_x + kRect.x + 2, .y = tile_y + kRect.y}
        };
      })
  }
{
  static constexpr auto kBackgroundAlpha = 175U;
  SDL_SetTextureBlendMode(all_tiles_.get(), SDL_BLENDMODE_BLEND);
  SDL_SetTextureBlendMode(unavailable_shadows_.get(), SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, unavailable_shadows_.get());
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_SetRenderTarget(renderer, all_tiles_.get());
  SDL_SetRenderDrawColor(renderer, constants::kFontColorBrown.r, constants::kFontColorBrown.g, constants::kFontColorBrown.b, kBackgroundAlpha);
  SDL_RenderClear(renderer);
  for(SDL_Rect tile_rect{0, 0, kTileW, kTileH }; auto [texture, counter] : std::views::zip(unique_textures, texts_)) {
    tile_rect.x = counter.x() - kRect.x -2; tile_rect.y = counter.y() - kRect.y;
    SDL_RenderCopy(renderer, texture, nullptr, &tile_rect);
  }
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_SetRenderTarget(renderer, nullptr);
  utility::log("Unplayed tiles counter initialized");
}

void UnplayedTileCounter::update_count(SDL_Renderer *renderer, std::span<const Tile> recently_placed) {
  static constexpr SDL_Color shadow{.r = 0, .g = 0, .b = 0, .a = 100};
  static constexpr auto get_idx = [](const Tile& tile){
    return tile.value == 0 ? constants::kNumOfTiles-1 : static_cast<size_t>(tile.letter - 'a');
  };
  for (const Tile& tile : recently_placed) {
    --infos_[get_idx(tile)].frequency;
  }
  for (const Tile& tile : recently_placed) {
    int new_freq = infos_[get_idx(tile)].frequency;
    Text &text = texts_[get_idx(tile)];
    if (new_freq == 0) {
      SDL_Rect black_shadow{.x = text.x() - kRect.x - 2, .y = text.y() - kRect.y, .w = kTileW, .h = kTileH};
      SDL_SetRenderTarget(renderer, unavailable_shadows_.get());
      SDL_SetRenderDrawColor(renderer, shadow.r, shadow.g, shadow.b, shadow.a);
      SDL_RenderFillRect(renderer, &black_shadow);
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_SetRenderTarget(renderer, nullptr);
    }
    text.update(renderer, std::to_string(new_freq));
  }
}

void UnplayedTileCounter::reset(SDL_Renderer* renderer) {
  for(auto [current, original, text] : std::views::zip(infos_, constants::tile_info, texts_)) {
    if(current.frequency != original.frequency){
      text.update(renderer, std::to_string(original.frequency));
      current.frequency = original.frequency;
    }
  }
  SDL_SetRenderTarget(renderer, unavailable_shadows_.get());
  SDL_RenderClear(renderer);
  SDL_SetRenderTarget(renderer, nullptr);
}

void UnplayedTileCounter::render(SDL_Renderer* renderer) const {
  SDL_RenderCopy(renderer, all_tiles_.get(), nullptr, &kRect);
  for(const auto& text : texts_) {
    text.render(renderer);
  }
  SDL_RenderCopy(renderer, unavailable_shadows_.get(), nullptr, &kRect);
}
