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

template <std::ranges::contiguous_range R>
constexpr auto iterator_at(R& range, size_t index) {
  assert(index < std::ranges::size(range));
  return std::next(range.begin(), static_cast<typename R::difference_type>(index));
}

consteval size_t total_num_of_tiles(){
  return std::ranges::fold_left(std::views::transform(constants::tile_info, [](constants::TileInfo info){
        return info.frequency; }), 0, std::plus{});
}

inline Surface surface_from_idx(size_t idx) {
  return Surface{
    IMG_LoadWEBP_RW(RWops{SDL_RWFromConstMem(tile_files[idx].data(), static_cast<int>(tile_files[idx].size()))}.get())
  };
}

} // namespace

TileBag::TileBag(SDL_Renderer* renderer) :
  TileBag(renderer, surface_from_idx(0))
{
  utility::log("Tile bag initialized");
}

/*
 * The reason for this complicated initialization is that Tile does not have a
 * default constructor, as well as tiles each having a different frequency. We
 * thus need to generate textures and tiles using special generating functions
 * that will eventually populate std::arrays (keeping in mind that we cannot
 * make make a std::array with non-default constructible elements without
 * aggregate initialization)
 */
TileBag::TileBag(SDL_Renderer* renderer, Surface current_surface) :
  tile_textures_{
    utility::generate_array<total_num_of_tiles()>(
      [
        renderer,
        &current_surface,
        curr_idx = 0UZ,
        curr_freq = 0
      ] mutable {
        if(curr_freq == constants::tile_info[curr_idx].frequency) {
          ++curr_idx;
          curr_freq = 0;
          current_surface = surface_from_idx(curr_idx);
        }
        ++curr_freq;
        return Texture{SDL_CreateTextureFromSurface(renderer, current_surface.get())};
      }
    )
  },
  tile_bag_{
    utility::generate_array<total_num_of_tiles()>(
      [
        this,
        curr_idx = 0UZ,
        curr_freq = 0,
        curr_texture_idx = 0UZ
      ] mutable {
        if(curr_freq == constants::tile_info[curr_idx].frequency) {
          ++curr_idx;
          curr_freq = 0;
        }
        ++curr_freq;
        return Tile(tile_textures_[curr_texture_idx++].get(),
                    SDL_Rect{.x=0, .y=0, .w=constants::kTileWidth, .h=constants::kTileHeight},
                    constants::tile_info[curr_idx].letter,
                    constants::tile_info[curr_idx].value);
      }
    )
  }
{}

[[nodiscard]] Tile TileBag::take_from() {
  assert(tiles_left() > 0);
  return tile_bag_[take_from_index_++];
}

[[nodiscard]] size_t TileBag::tiles_left() const {
  assert(take_from_index_ <= tile_bag_.size());
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
  std::ranges::shuffle(current_take_from, tile_bag_.end(), utility::random::engine());
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
  std::ranges::shuffle(iterator_at(tile_bag_, take_from_index_), tile_bag_.end(), utility::random::engine());
}
