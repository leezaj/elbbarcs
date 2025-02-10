#include "battery/embed.hpp"
#include "BlankTileReplacer.h"
#include "Board.h"
#include "Playing.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <cassert>

namespace {
struct BlankTileInfo {
  b::EmbedInternal::EmbeddedFile file;
  char letter;
};
} // namespace

void BlankTileReplacer::load_tiles(SDL_Renderer* renderer) {
  std::array<BlankTileInfo, constants::kNumOfTiles-1> blank_infos{{
      {b::embed<"assets/blank-tiles/a.png">(),'a'},
      {b::embed<"assets/blank-tiles/b.png">(),'b'},
      {b::embed<"assets/blank-tiles/c.png">(),'c'},
      {b::embed<"assets/blank-tiles/d.png">(),'d'},
      {b::embed<"assets/blank-tiles/e.png">(),'e'},
      {b::embed<"assets/blank-tiles/f.png">(),'f'},
      {b::embed<"assets/blank-tiles/g.png">(),'g'},
      {b::embed<"assets/blank-tiles/h.png">(),'h'},
      {b::embed<"assets/blank-tiles/i.png">(),'i'},
      {b::embed<"assets/blank-tiles/j.png">(),'j'},
      {b::embed<"assets/blank-tiles/k.png">(),'k'},
      {b::embed<"assets/blank-tiles/l.png">(),'l'},
      {b::embed<"assets/blank-tiles/m.png">(),'m'},
      {b::embed<"assets/blank-tiles/n.png">(),'n'},
      {b::embed<"assets/blank-tiles/o.png">(),'o'},
      {b::embed<"assets/blank-tiles/p.png">(),'p'},
      {b::embed<"assets/blank-tiles/q.png">(),'q'},
      {b::embed<"assets/blank-tiles/r.png">(),'r'},
      {b::embed<"assets/blank-tiles/s.png">(),'s'},
      {b::embed<"assets/blank-tiles/t.png">(),'t'},
      {b::embed<"assets/blank-tiles/u.png">(),'u'},
      {b::embed<"assets/blank-tiles/v.png">(),'v'},
      {b::embed<"assets/blank-tiles/w.png">(),'w'},
      {b::embed<"assets/blank-tiles/x.png">(),'x'},
      {b::embed<"assets/blank-tiles/y.png">(),'y'},
      {b::embed<"assets/blank-tiles/z.png">(),'z'},
  }};
  auto textures = utility::map(blank_infos, [renderer](const BlankTileInfo& info) -> Texture {
    SDL_RWops *buffer = SDL_RWFromConstMem(info.file.data(), static_cast<int>(info.file.size()));
    return Texture{IMG_LoadTexture_RW(renderer, buffer, 1)};
  });
  std::vector<Tile> tiles;
  tiles.reserve(constants::kNumOfTiles-1);
  std::ranges::transform(blank_infos, textures, std::back_inserter(tiles), [](const BlankTileInfo& i, const Texture& t){
    return Tile(t.get(), SDL_Rect{.x=0, .y=0, .w=constants::kTileWidth, .h=constants::kTileHeight}, i.letter, 0);
  });
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
