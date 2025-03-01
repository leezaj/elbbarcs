#include "battery/embed.hpp"
#include "BlankTileReplacer.h"
#include "Board.h"
#include "Playing.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <cassert>

BlankTileReplacer::BlankTileReplacer(SDL_Renderer *rend, Mouse& mouse, GameStateManager& manager, Board& board, Playing& playing_state) : 
  GameState(rend, mouse),
  state_manager_{&manager},
  board_{&board},
  playing_state_{&playing_state}
{
  const std::array blank_infos{
      b::embed<"assets/blank-tiles/a.png">(),
      b::embed<"assets/blank-tiles/b.png">(),
      b::embed<"assets/blank-tiles/c.png">(),
      b::embed<"assets/blank-tiles/d.png">(),
      b::embed<"assets/blank-tiles/e.png">(),
      b::embed<"assets/blank-tiles/f.png">(),
      b::embed<"assets/blank-tiles/g.png">(),
      b::embed<"assets/blank-tiles/h.png">(),
      b::embed<"assets/blank-tiles/i.png">(),
      b::embed<"assets/blank-tiles/j.png">(),
      b::embed<"assets/blank-tiles/k.png">(),
      b::embed<"assets/blank-tiles/l.png">(),
      b::embed<"assets/blank-tiles/m.png">(),
      b::embed<"assets/blank-tiles/n.png">(),
      b::embed<"assets/blank-tiles/o.png">(),
      b::embed<"assets/blank-tiles/p.png">(),
      b::embed<"assets/blank-tiles/q.png">(),
      b::embed<"assets/blank-tiles/r.png">(),
      b::embed<"assets/blank-tiles/s.png">(),
      b::embed<"assets/blank-tiles/t.png">(),
      b::embed<"assets/blank-tiles/u.png">(),
      b::embed<"assets/blank-tiles/v.png">(),
      b::embed<"assets/blank-tiles/w.png">(),
      b::embed<"assets/blank-tiles/x.png">(),
      b::embed<"assets/blank-tiles/y.png">(),
      b::embed<"assets/blank-tiles/z.png">(),
  };
  blanks_textures_ = utility::map(blank_infos, [rend](const auto& file) -> Texture {
    SDL_RWops *buffer = SDL_RWFromConstMem(file.data(), static_cast<int>(file.size()));
    return Texture{IMG_LoadTexture_RW(rend, buffer, 1)};
  });
  blanks_tiles_.reserve(constants::kNumOfTiles-1);
  for(auto [texture, info] : std::views::zip(blanks_textures_, constants::tile_info)) {
    blanks_tiles_.emplace_back(texture.get(), 
        SDL_Rect{.x = 0, .y = 0, .w = constants::kTileWidth, .h = constants::kTileHeight}, info.letter, 0);
  }
  static constexpr auto rows = 4;
  static constexpr auto tiles_per_row = constants::kNumOfTiles / rows;
  static constexpr auto begin_x = 100, begin_y = 200, gap = 100;
  // TODO: replace with std::views::enumerate once LLVM supports it
  for(auto idx = 0; auto& tile : blanks_tiles_) {
    tile.move(SDL_Point{begin_x + gap * (idx % tiles_per_row), begin_y + gap * (idx / tiles_per_row)});
    ++idx;
  }
  utility::log("Blank tile replacer initialized");
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
