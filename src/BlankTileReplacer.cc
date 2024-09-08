#include "AssetPool.h"
#include "BlankTileReplacer.h"
#include "Board.h"
#include "Playing.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <cassert>

namespace {

const auto blanks_path = AssetPool::assets_path()/"blank-tiles";

} //namespace

void BlankTileReplacer::load_tiles(SDL_Renderer* renderer) {
  std::vector<std::filesystem::path> files{std::filesystem::directory_iterator(blanks_path), {}};
  std::ranges::sort(files);
  assert(files.size() == constants::kNumOfTiles-1);
  std::vector<Texture> textures;
  textures.reserve(constants::kNumOfTiles-1);
  std::vector<Tile> tiles;
  tiles.reserve(constants::kNumOfTiles-1);
  for(const auto& file: files){
    std::string_view filename{file.stem().c_str()};
    assert(filename.size() == 1);
    textures.emplace_back(IMG_LoadTexture(renderer, file.c_str()));
    tiles.emplace_back(textures.back().get(), SDL_Rect{0, 0, constants::kTileWidth, constants::kTileHeight}, filename.front(), 0);
  }
  blanks_textures_ = std::move(textures);
  blanks_tiles_ = std::move(tiles);
}

BlankTileReplacer::BlankTileReplacer(SDL_Renderer *rend, Mouse& mouse, GameStateManager& manager, Board& board, Playing& playing_state) : 
  GameState(rend, mouse),
  state_manager_{&manager},
  board_{&board},
  playing_state_{&playing_state}
{
  load_tiles(rend);
  static constexpr auto rows = 4;
  static constexpr auto tiles_per_row = constants::kNumOfTiles / rows;
  static constexpr auto begin_x = 100, begin_y = 200, gap = 100;
  // TODO: replace with std::views::enumerate once LLVM supports it
  for(auto idx = 0; auto& tile : blanks_tiles_) {
    tile.move(SDL_Point{begin_x + gap * (idx % tiles_per_row), begin_y + gap * (idx / tiles_per_row)});
    ++idx;
  }
}

void BlankTileReplacer::render_objects() const {
  SDL_RenderCopy(renderer(), bg_texture_.get(), nullptr, nullptr);
  std::ranges::for_each(blanks_tiles_, [this](const auto &tile) { tile.render(renderer()); });
}

void BlankTileReplacer::handle_event(const SDL_Event& event) {
  switch(event.type) {
  case SDL_MOUSEMOTION:
    set_mouse_pos({.x=event.motion.x, .y=event.motion.y});
    hovered_tile_ = handle_hovering(blanks_tiles_);
    return;
  case SDL_MOUSEBUTTONUP:
    if (hovered_tile_ == nullptr) {
      return;
    }
    hovered_tile_->unhover();
    board_->put_on_board(*hovered_tile_);
    set_default_cursor();
    hovered_tile_ = nullptr;
    state_manager_->pop();
    playing_state_->evaluate_board();
    return;
  default:
    return;
  }
}

void BlankTileReplacer::replace_blank() {
  Texture bg = playing_state_->get_snapshot();
  SDL_SetTextureColorMod(bg.get(), constants::kDimRGB, constants::kDimRGB, constants::kDimRGB);
  bg_texture_ = std::move(bg);
  state_manager_->push(this);
  hovered_tile_ = handle_hovering(blanks_tiles_);
}

void BlankTileReplacer::replace_blank(Row_Col row_col, char letter) {
  assert(letter >= 'a' and letter <= 'z');
  board_->put_on_board(row_col, blanks_tiles_[static_cast<size_t>(letter - 'a')], false);
}
