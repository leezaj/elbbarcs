#include "battery/embed.hpp"
#include "Tile.h"
#include "TileBag.h"
#include "constants.h"
#include "utility.h"
#include <SDL2/SDL_render.h>
#include <SDL_image.h>
#include <algorithm>
#include <cassert>
#include <print>
#include <ranges>

namespace {
  template <std::ranges::contiguous_range R>
  constexpr auto iterator_at(R& range, size_t index) {
    return std::next(range.begin(), static_cast<typename R::difference_type>(index));
  }
} // namespace

TileBag::TileBag(SDL_Renderer* renderer)
{
  std::array<b::EmbedInternal::EmbeddedFile, constants::kNumOfTiles> files{{
      b::embed<"assets/tiles/a.png">(),
      b::embed<"assets/tiles/b.png">(),
      b::embed<"assets/tiles/c.png">(),
      b::embed<"assets/tiles/d.png">(),
      b::embed<"assets/tiles/e.png">(),
      b::embed<"assets/tiles/f.png">(),
      b::embed<"assets/tiles/g.png">(),
      b::embed<"assets/tiles/h.png">(),
      b::embed<"assets/tiles/i.png">(),
      b::embed<"assets/tiles/j.png">(),
      b::embed<"assets/tiles/k.png">(),
      b::embed<"assets/tiles/l.png">(),
      b::embed<"assets/tiles/m.png">(),
      b::embed<"assets/tiles/n.png">(),
      b::embed<"assets/tiles/o.png">(),
      b::embed<"assets/tiles/p.png">(),
      b::embed<"assets/tiles/q.png">(),
      b::embed<"assets/tiles/r.png">(),
      b::embed<"assets/tiles/s.png">(),
      b::embed<"assets/tiles/t.png">(),
      b::embed<"assets/tiles/u.png">(),
      b::embed<"assets/tiles/v.png">(),
      b::embed<"assets/tiles/w.png">(),
      b::embed<"assets/tiles/x.png">(),
      b::embed<"assets/tiles/y.png">(),
      b::embed<"assets/tiles/z.png">(),
      b::embed<"assets/tiles/blank_tile.png">()
  }};
  size_t current_idx = 0;
  for(const auto& [asset, tile]: std::views::zip(files, constants::tile_info)) {
    RWops buffer {SDL_RWFromConstMem(asset.data(), static_cast<int>(asset.size()))};
    Surface temp{IMG_Load_RW(buffer.get(), 0)};
    for (auto i = 0; i < tile.frequency; ++i) {
      tile_textures_[current_idx] = Texture(SDL_CreateTextureFromSurface(renderer, temp.get()));
      tile_bag_[current_idx] = Tile(tile_textures_[current_idx].get(), 
          SDL_Rect{ .x = 0, .y = 0, .w = constants::kTileWidth, .h = constants::kTileHeight}, tile.letter, tile.value);
      ++current_idx;
    }
  }
  utility::log("Tile bag initialized");
}

[[nodiscard]] Tile TileBag::take_from() {
  return tile_bag_[take_from_index_++];
}

[[nodiscard]] size_t TileBag::tiles_left() const {
  return tile_bag_.size() - take_from_index_;
}

[[nodiscard]] std::span<const Tile> TileBag::tiles_view() const {
  return tile_bag_;
}

[[nodiscard]] std::vector<Tile> TileBag::swap(std::span<const Tile> with) {
  assert(tiles_left() >= with.size());
  std::vector<Tile> result;
  result.reserve(with.size());
  auto current = iterator_at(tile_bag_, take_from_index_);
  auto lookahead = current;
  for(const Tile& existing: with) {
    auto it = std::ranges::find(tile_bag_.begin(), current, existing.letter(), &Tile::letter);
    assert(it != tile_bag_.end());
    result.push_back(*lookahead);
    std::iter_swap(it, lookahead);
    std::advance(lookahead, 1);
    assert(lookahead != tile_bag_.end());
  }
  std::ranges::shuffle(current, tile_bag_.end(), Random::engine());
  return result;
}

void TileBag::reset() {
  take_from_index_ = 0;
  shuffle();
}

[[nodiscard]] bool TileBag::empty() const {
  return take_from_index_ == tile_bag_.size();
}

void TileBag::shuffle() {
  const auto current = iterator_at(tile_bag_, take_from_index_);
  std::ranges::shuffle(current, tile_bag_.end(), Random::engine());
}
