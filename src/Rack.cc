#include "Rack.h"
#include "utility.h"
#include <SDL2/SDL_render.h>
#include <cassert>
#include <cstddef>
#include <ranges>
#include <utility>

namespace {
// check if the mouse is within the top, left and right of the rack
constexpr bool contains_ignoring_bottom(const SDL_Rect& rect, SDL_Point point){
  return point.x > rect.x and point.x < (rect.x + rect.w) and point.y >= rect.y;
}

auto get_first_gap(auto &container) {
  return std::ranges::find(container, constants::kTileGapChar, &Tile::letter);
}

auto get_first_tile(auto &container) {
  return std::ranges::find_if_not(container, std::bind_front(std::equal_to{}, constants::kTileGapChar), &Tile::letter);
}

std::uint8_t index_of(auto& container, const auto& it) {
  return static_cast<std::uint8_t>(std::distance(container.begin(), it));
}
} // namespace

Rack::Rack(SDL_Renderer *renderer) : 
  missing_tile_texture_{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, constants::kTileWidth, constants::kTileHeight)},
  tiles_{utility::map<utility::to_array>(constants::kRackTilePositions, [this](SDL_Point point){ return create_gap_at(point); })}
{
  SDL_SetTextureBlendMode(missing_tile_texture_.get(), SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, missing_tile_texture_.get());
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_SetRenderTarget(renderer, nullptr);
  utility::log("Rack initialized");
}

auto Rack::get_tile_idx(SDL_Point point) {
  static constexpr double space_per_tile = rect_.w / static_cast<double>(constants::kRackTileAmount);
  auto index = static_cast<std::uint8_t>((point.x - rect_.x) / space_per_tile);
  assert(index < constants::kRackTileAmount);
  return index;
}

Tile Rack::create_gap_at(SDL_Point point) {
  return Tile{missing_tile_texture_.get(), 
    SDL_Rect{.x = point.x, .y = point.y, .w = constants::kTileWidth, .h = constants::kTileHeight},
    constants::kTileGapChar, 0};
}

void Rack::render(SDL_Renderer* renderer) const noexcept {
  std::ranges::for_each(tiles_,[=](const auto& obj){obj.render(renderer);});
}

void Rack::put(Tile tile) {
  auto gap = get_first_gap(tiles_);
  assert(gap != tiles_.end());
  tile.move(constants::kRackTilePositions[index_of(tiles_, gap)]);
  *gap = tile;
}

Tile *Rack::find_tile(SDL_Point point) {
  if (not contains(rect_, point)) {
    return nullptr;
  }
  Tile& tile = tiles_[get_tile_idx(point)];
  return (tile.letter != constants::kTileGapChar and contains(tile.rect, point)) ? &tile : nullptr;
}

Tile* Rack::take_from(SDL_Point point) {
  if(Tile* it = find_tile(point); it!=nullptr){
    taken_ = std::exchange(*it, create_gap_at(it->point()));
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
  std::ranges::shuffle(tiles_, Random::engine());
  for(auto [tile, position] : std::views::zip(tiles_, constants::kRackTilePositions)) {
    tile.move(position);
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

Tile Rack::take_tile(std::uint32_t idx) {
  assert(idx<tiles_.size() and tiles_[idx].letter != constants::kTileGapChar);
  return std::exchange(tiles_[idx], create_gap_at(tiles_[idx].point()));
}

Tile Rack::take_tile(char letter) {
  auto it = std::ranges::find(tiles_, letter, &Tile::letter);
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

void Rack::reset() {
  for(Tile& tile : tiles_) {
    tile = create_gap_at(tile.point());
  }
}
