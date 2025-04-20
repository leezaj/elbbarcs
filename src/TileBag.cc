#include "battery/embed.hpp"
#include "Tile.h"
#include "TileBag.h"
#include "utility.h"
#include <SDL2/SDL_render.h>
#include <SDL_image.h>
#include <algorithm>
#include <cassert>
#include <ranges>

namespace {
constexpr std::array tile_files{
    b::embed<"assets/tiles/a.webp">(),
    b::embed<"assets/tiles/b.webp">(),
    b::embed<"assets/tiles/c.webp">(),
    b::embed<"assets/tiles/d.webp">(),
    b::embed<"assets/tiles/e.webp">(),
    b::embed<"assets/tiles/f.webp">(),
    b::embed<"assets/tiles/g.webp">(),
    b::embed<"assets/tiles/h.webp">(),
    b::embed<"assets/tiles/i.webp">(),
    b::embed<"assets/tiles/j.webp">(),
    b::embed<"assets/tiles/k.webp">(),
    b::embed<"assets/tiles/l.webp">(),
    b::embed<"assets/tiles/m.webp">(),
    b::embed<"assets/tiles/n.webp">(),
    b::embed<"assets/tiles/o.webp">(),
    b::embed<"assets/tiles/p.webp">(),
    b::embed<"assets/tiles/q.webp">(),
    b::embed<"assets/tiles/r.webp">(),
    b::embed<"assets/tiles/s.webp">(),
    b::embed<"assets/tiles/t.webp">(),
    b::embed<"assets/tiles/u.webp">(),
    b::embed<"assets/tiles/v.webp">(),
    b::embed<"assets/tiles/w.webp">(),
    b::embed<"assets/tiles/x.webp">(),
    b::embed<"assets/tiles/y.webp">(),
    b::embed<"assets/tiles/z.webp">(),
    b::embed<"assets/tiles/blank_tile.webp">()
};
} // namespace

namespace {
  template <std::ranges::contiguous_range R>
  constexpr auto iterator_at(R& range, size_t index) {
    assert(index < std::ranges::size(range));
    return std::next(range.begin(), static_cast<typename R::difference_type>(index));
  }
} // namespace

TileBag::TileBag(SDL_Renderer* renderer)
{
  size_t current_idx = 0;
  for(const auto& [asset, tile]: std::views::zip(tile_files, constants::tile_info)) {
    RWops buffer {SDL_RWFromConstMem(asset.data(), static_cast<int>(asset.size()))};
    Surface temp{IMG_LoadWEBP_RW(buffer.get())};
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
  assert(tiles_left() > 0);
  return tile_bag_[take_from_index_++];
}

[[nodiscard]] size_t TileBag::tiles_left() const {
  assert(take_from_index_ < tile_bag_.size());
  return tile_bag_.size() - take_from_index_;
}

[[nodiscard]] std::span<const Tile> TileBag::tiles_view() const {
  return tile_bag_;
}

[[nodiscard]] std::vector<Tile> TileBag::swap(std::span<const Tile> with) {
  assert(tiles_left() >= with.size());
  std::vector<Tile> result;
  result.reserve(with.size());
  auto current_take_from = iterator_at(tile_bag_, take_from_index_);
  auto lookahead = current_take_from;
  for(const Tile& existing: with) {
    auto it = std::ranges::find(tile_bag_.begin(), current_take_from, existing.letter, &Tile::letter);
    assert(it != tile_bag_.end());
    result.push_back(*lookahead);
    std::iter_swap(it, lookahead);
    std::advance(lookahead, 1);
    assert(lookahead != tile_bag_.end());
  }
  std::ranges::shuffle(current_take_from, tile_bag_.end(), Random::engine());
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
  std::ranges::shuffle(iterator_at(tile_bag_, take_from_index_), tile_bag_.end(), Random::engine());
}
