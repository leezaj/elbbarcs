#include "Board.h"
#include "BlankTileReplacer.h"
#include "constants.h"
#include "utility.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <cassert>
#include <ranges>
#include <utility>
#include <vector>

constexpr SDL_Rect kTrimmedTile{0, 0, constants::kTileWidth, constants::kTileHeight - 3};

constexpr SDL_Rect kRect{0, 0, constants::kBoardDims, constants::kBoardDims};

Board::Board(SDL_Renderer *renderer, BlankTileReplacer& blank_replacer) : 
  renderer_{renderer},
  replacer_{&blank_replacer},
  board_texture_{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, kRect.w, kRect.h)}
{
  // initialize the board texture's pixels to be completely transparent
  SDL_SetTextureBlendMode(board_texture_.get(), SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, board_texture_.get());
  SDL_RenderClear(renderer);
  SDL_SetRenderTarget(renderer, nullptr);
  recently_placed_.reserve(constants::kRackTileAmount);
  already_played_.reserve(constants::kBagTileAmount);
  computer_highlighted_tiles_.reserve(constants::kBagTileAmount);
}

bool Board::has_recently_placed_tiles() const {
  return not recently_placed_.empty() or not blanks_.empty();
}

std::vector<Row_Col> Board::placed_tile_positions() const {
  return  recently_placed_ |
    std::views::transform(&Tile::point) | 
    std::views::transform(&to_row_col) |
    std::ranges::to<std::vector>();
}

bool Board::empty() const {
  return recently_placed_.empty() && blanks_.empty() && already_played_.empty();
}

size_t Board::num_of_tiles() const {
  return recently_placed_.size() + already_played_.size();
}

const Tile* Board::tile_at_pos(SDL_Point point) const {
  auto it = std::ranges::find_if(recently_placed_, [=](const Tile &tile) { return contains(tile.rectangle(), point);}); 
  return it != recently_placed_.end() ? &(*it) : nullptr;
}

void Board::place_highlight(const SDL_Rect& rect, bool is_player) {
  static constexpr SDL_Color kPlayerHighlight{175, 50, 0, 30};
  static constexpr SDL_Color kComputerHighlight{150, 100, 0, 30};
  const auto to_highlight = (is_player) ? kPlayerHighlight : kComputerHighlight;
  SDL_SetRenderTarget(renderer_, board_texture_.get());
  SDL_SetRenderDrawColor(renderer_, to_highlight.r, to_highlight.g, to_highlight.b, to_highlight.a);
  SDL_RenderFillRect(renderer_, &rect);
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
  SDL_SetRenderTarget(renderer_, nullptr);
}

bool Board::put_on_board(const Tile& tile) {
  if (not shadow_) {
    return false;
  }
  const char letter = tile.letter();
  const std::uint8_t value = tile.value();
  model_.set_square(to_row_col({shadow_->x, shadow_->y}), letter, value);
  if(value==0){
    if(letter != constants::kTileBlankChar) {
      blanks_.push_back(recently_placed_.back());
      recently_placed_.pop_back();
    } else {
      recently_placed_.emplace_back(tile.texture(), *shadow_, letter, value);
      clear_rect(*shadow_);
      replacer_->replace_blank();
      return true;
    }
  }
  recently_placed_.emplace_back(tile.texture(), *shadow_, letter, value);
  place_highlight(*shadow_, true);
  shadow_.reset();
  return true;
}

bool Board::put_on_board(Row_Col pos, Tile tile, bool is_player) {
  assert(not model_.is_filled(pos));
  SDL_Point point = to_point(pos);
  tile.move(point);
  tile.set_dimensions({.w = constants::kSquarePixelSize, .h = constants::kSquarePixelSize});
  if(tile.letter() == constants::kTileBlankChar){
    blanks_.emplace_back(tile);
  } else {
    model_.set_square(pos, tile.letter(), tile.value());
    recently_placed_.emplace_back(tile);
  }
  if (is_player) {
    place_highlight(tile.rectangle(), is_player);
  }
  return true;
}

Tile *Board::take_from_board(SDL_Point point) {
  if(const Tile* const it = tile_at_pos(point); it!=nullptr){
    if(it->value() == 0){
      const auto blank = std::ranges::find(blanks_, it->point(), &Tile::point);
      assert(blank!=blanks_.end());
      taken_ = *blank;
      blanks_.erase(blank);
    } else {
      taken_ = *it;
    }
    clear_rect(taken_.rectangle());
    taken_.set_dimensions({constants::kTileWidth, constants::kTileHeight});
    model_.clear_square(to_row_col(taken_.point()));
    // because it is a pointer to const, we need to make the beginning of the
    // array a pointer to const as well to make it work with std::distance
    recently_placed_.erase(recently_placed_.begin() + std::distance(std::as_const(recently_placed_).data(), it));
    last_taken_point_ = taken_.point();
    return &(taken_);
  }
  return nullptr;
}

void Board::return_tile() {
  put_shadow(last_taken_point_);
  put_on_board(taken_);
}

Tile Board::take_oldest_placed() {
  assert(not recently_placed_.empty());
  return *take_from_board(recently_placed_.front().point());
}

Tile Board::take_tile() {
  assert(not recently_placed_.empty() or not already_played_.empty());
  if (recently_placed_.empty()) {
    Tile back = already_played_.back();
    already_played_.pop_back();
    return back;
  }
  return *take_from_board(recently_placed_.back().point());
}

void Board::reset() {
  model_.reset();
  shadow_.reset();
  computer_highlighted_tiles_.clear();
  SDL_SetRenderTarget(renderer_, board_texture_.get());
  SDL_RenderClear(renderer_);
  SDL_SetRenderTarget(renderer_, nullptr);
}

void Board::clear_rect(const SDL_Rect& rect) {
  SDL_SetRenderTarget(renderer_, board_texture_.get());
  SDL_RenderFillRect(renderer_, &rect);
  SDL_SetRenderTarget(renderer_, nullptr);
}

void Board::put_shadow(SDL_Point point) {
  if (shadow_) {
    if (contains(*shadow_, point)) {
      return;
    }
    clear_rect(*shadow_);
    shadow_.reset();
  }
  if (contains(kRect, point) and
      model_.is_empty(to_row_col(point))) {
    shadow_ = SDL_Rect{.x = (point.x / constants::kSquarePixelSize) * constants::kSquarePixelSize,
                       .y = (point.y / constants::kSquarePixelSize) * constants::kSquarePixelSize,
                       .w = constants::kSquarePixelSize,
                       .h = constants::kSquarePixelSize};
    static constexpr std::uint8_t kShadowOpacity = 80;
    SDL_SetRenderTarget(renderer_, board_texture_.get());
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, kShadowOpacity);
    SDL_RenderFillRect(renderer_, &(*shadow_));
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
    SDL_SetRenderTarget(renderer_, nullptr);
  }
}

void Board::play_placed_tiles(bool is_player) {
  SDL_SetRenderTarget(renderer_, board_texture_.get());
  if (is_player) {
    for (const auto &tile : recently_placed_) {
      SDL_RenderCopy(renderer_, tile.texture(), &kTrimmedTile, &tile.rectangle());
    }
  } else {
    for (const auto &tile : computer_highlighted_tiles_) {
      SDL_RenderCopy(renderer_, tile.texture(), &kTrimmedTile, &tile.rectangle());
    }
    computer_highlighted_tiles_ = recently_placed_;
    for (const auto &tile : computer_highlighted_tiles_) {
      place_highlight(tile.rectangle(), false);
    }
  }
  SDL_SetRenderTarget(renderer_, nullptr);
  for(auto& blank: blanks_){
    blank.set_dimensions({.w=constants::kTileWidth, .h=constants::kTileHeight});
    already_played_.push_back(blank);
  }
  for(auto& tile: recently_placed_){
    model_.set_played(to_row_col(tile.point()));
    if (tile.value() == 0) {
      continue;
    }
    tile.set_dimensions({.w=constants::kTileWidth, .h=constants::kTileHeight});
    already_played_.push_back(tile);
  }
  recently_placed_.clear();
  blanks_.clear();
}

void Board::render() const {
  static const auto render_trimmed_tile = [this](const Tile &tile) {
    SDL_RenderCopy(renderer_, tile.texture(), &kTrimmedTile, &tile.rectangle());
  };
  std::ranges::for_each(recently_placed_,  render_trimmed_tile);
  std::ranges::for_each(computer_highlighted_tiles_, render_trimmed_tile);
  SDL_RenderCopy(renderer_, board_texture_.get(), nullptr, &kRect);
}
