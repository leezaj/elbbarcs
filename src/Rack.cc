#include "Rack.h"
#include "utility.h"
#include <SDL2/SDL_render.h>
#include <cassert>
#include <cstddef>
#include <utility>

namespace {
// check if the mouse is within the top, left and right of the rack
constexpr bool contains_ignoring_bottom(const SDL_Rect& rect, SDL_Point point){
  return point.x > rect.x and point.x < (rect.x + rect.w) and point.y >= rect.y;
}

auto get_first_gap(auto &cont) {
  return std::ranges::find(cont, constants::kTileGapChar, &Tile::letter);
}

auto get_first_tile(auto &cont) {
  return std::ranges::find_if_not( cont, std::bind_front(std::equal_to{}, constants::kTileGapChar), &Tile::letter);
}

std::uint8_t index_of(auto& cont, const auto& it) {
  return static_cast<std::uint8_t>(std::distance(cont.begin(), it));
}
} // namespace

Rack::Rack(SDL_Renderer *renderer) : 
  missing_tile_texture_{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, constants::kTileWidth, constants::kTileHeight)} 
{
  SDL_SetTextureBlendMode(missing_tile_texture_.get(), SDL_BLENDMODE_BLEND);
  // update the gap texture's pixels with zeroed out pixels so it's transparent
  const std::array<std::uint32_t, static_cast<std::size_t> (constants::kTileWidth * constants::kTileHeight)> pixels{};
  SDL_UpdateTexture(missing_tile_texture_.get(), nullptr, pixels.data(), sizeof(std::uint32_t) * constants::kTileWidth);
}

auto Rack::get_tile_idx(SDL_Point point) {
  static constexpr double space_per_tile = rect_.w / static_cast<double>(constants::kRackTileAmount);
  return static_cast<std::uint8_t>((point.x - rect_.x) / space_per_tile);
}

Tile Rack::create_gap_at(SDL_Point point) {
  return Tile{missing_tile_texture_.get(), SDL_Rect{.x = point.x,
                       .y = point.y,
                       .w = constants::kTileWidth,
                       .h = constants::kTileHeight},
              constants::kTileGapChar, 0};
}

void Rack::render(SDL_Renderer* renderer) const noexcept {
  std::ranges::for_each(tiles_,[=](const auto& obj){obj.render(renderer);});
}

void Rack::put(Tile tile) {
  auto *gap = get_first_gap(tiles_);
  tile.move(constants::kRackTilePositions[index_of(tiles_, gap)]);
  *gap = tile;
}

Tile *Rack::find_tile(SDL_Point point) {
  if (not contains(rect_, point)) {
    return nullptr;
  }
  Tile& tile = tiles_[get_tile_idx(point)];
  return (tile.letter() != constants::kTileGapChar and contains(tile.rectangle(), point)) ? &tile : nullptr;
}

const Tile* Rack::tile_at_pos(SDL_Point point) noexcept{
  return find_tile(point);
}

Tile *Rack::take_from(SDL_Point point) {
  if(Tile* it = find_tile(point); it!=nullptr){
    taken_ = *it;
    *it = create_gap_at(it->point());
    taken_idx_ = static_cast<std::uint8_t>(std::distance(tiles_.data(), it));
    return &taken_;
  }
  return nullptr;
}

void Rack::return_tile() {
  tiles_[taken_idx_] = taken_;
  tiles_[taken_idx_].move(constants::kRackTilePositions[taken_idx_]);
}

void Rack::shuffle() {
  std::ranges::shuffle(tiles_, Random::engine);
  for(auto index = 0UZ; auto& tile: tiles_) {
    tile.move(constants::kRackTilePositions[index++]);
  }
}

void Rack::swap_tiles(SDL_Point point) {
  if (not contains_ignoring_bottom(rect_, point)) {
    return;
  }
  const auto held_idx = get_tile_idx(point);
  assert(held_idx < tiles_.size());
  Tile& swappee = tiles_[held_idx];
  swappee.move(constants::kRackTilePositions[taken_idx_]);
  tiles_[taken_idx_].move(constants::kRackTilePositions[held_idx]);
  std::swap(swappee, tiles_[taken_idx_]);
  taken_idx_ = held_idx;
}

void Rack::make_room_for_tile(SDL_Point mouse_point) {
  taken_idx_ = index_of(tiles_, get_first_gap(tiles_));
  swap_tiles(mouse_point);
}

int Rack::missing_tiles() const {
  return static_cast<int>(std::ranges::count(tiles_, constants::kTileGapChar, &Tile::letter));
}

size_t Rack::num_of_tiles() const {
  return tiles_.size() - static_cast<size_t>(missing_tiles());
}

Tile Rack::take_tile() {
  const auto idx = index_of(tiles_, get_first_tile(tiles_));
  return take_tile(static_cast<std::uint32_t>(idx));
}

Tile Rack::take_tile(std::uint32_t idx) {
  assert(idx<tiles_.size() and tiles_[idx].letter() != constants::kTileGapChar);
  return std::exchange(tiles_[idx], create_gap_at(tiles_[idx].point()));
}

Tile Rack::take_tile(char letter) {
  auto *it = std::ranges::find(tiles_, letter, &Tile::letter);
  assert(it!=tiles_.end());
  return take_tile(static_cast<uint32_t>(std::distance(tiles_.begin(), it)));
}

bool Rack::put(SDL_Point point, Tile tile) {
  if (not contains(rect_, point)) {
    return false;
  }
  const auto idx = get_tile_idx(point);
  tile.move(constants::kRackTilePositions[idx]);
  tiles_[idx] = tile;
  return true;
}
