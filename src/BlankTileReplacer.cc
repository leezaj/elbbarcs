#include "battery/embed.hpp"
#include "BlankTileReplacer.h"
#include "Board.h"
#include "Game.h"
#include "Mouse.h"
#include "Playing.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <cassert>

namespace {
constexpr std::array blank_infos{
    b::embed<"assets/blank-tiles/a.webp">(),
    b::embed<"assets/blank-tiles/b.webp">(),
    b::embed<"assets/blank-tiles/c.webp">(),
    b::embed<"assets/blank-tiles/d.webp">(),
    b::embed<"assets/blank-tiles/e.webp">(),
    b::embed<"assets/blank-tiles/f.webp">(),
    b::embed<"assets/blank-tiles/g.webp">(),
    b::embed<"assets/blank-tiles/h.webp">(),
    b::embed<"assets/blank-tiles/i.webp">(),
    b::embed<"assets/blank-tiles/j.webp">(),
    b::embed<"assets/blank-tiles/k.webp">(),
    b::embed<"assets/blank-tiles/l.webp">(),
    b::embed<"assets/blank-tiles/m.webp">(),
    b::embed<"assets/blank-tiles/n.webp">(),
    b::embed<"assets/blank-tiles/o.webp">(),
    b::embed<"assets/blank-tiles/p.webp">(),
    b::embed<"assets/blank-tiles/q.webp">(),
    b::embed<"assets/blank-tiles/r.webp">(),
    b::embed<"assets/blank-tiles/s.webp">(),
    b::embed<"assets/blank-tiles/t.webp">(),
    b::embed<"assets/blank-tiles/u.webp">(),
    b::embed<"assets/blank-tiles/v.webp">(),
    b::embed<"assets/blank-tiles/w.webp">(),
    b::embed<"assets/blank-tiles/x.webp">(),
    b::embed<"assets/blank-tiles/y.webp">(),
    b::embed<"assets/blank-tiles/z.webp">(),
};
} // namespace

BlankTileReplacer::BlankTileReplacer(Board& board, Playing& playing_state) : 
  blanks_textures_{utility::map<utility::to_array>(blank_infos, [](const auto& file) static -> Texture {
    RWops buffer{SDL_RWFromConstMem(file.data(), static_cast<int>(file.size()))};
    Surface surface{IMG_LoadWEBP_RW(buffer.get())};
    return Texture{SDL_CreateTextureFromSurface(Game::renderer(), surface.get())};
  })},
  blanks_tiles_{utility::map<utility::to_array>(blanks_textures_, [idx = 0] (const Texture& texture) mutable {
    static constexpr auto rows = 4;
    static constexpr auto tiles_per_row = constants::kNumOfTiles / rows;
    static constexpr auto begin_x = 100, begin_y = 200, gap = 100;
    return Tile{texture.get(), 
      SDL_Rect{
        .x = begin_x + (gap * (idx % tiles_per_row)), .y = begin_y + (gap * (idx / tiles_per_row)), 
        .w = constants::kTileWidth, .h = constants::kTileHeight
      }, static_cast<char>((idx++) + 'a'), 0
    };
  })},
  board_{&board},
  playing_state_{&playing_state}
{
  utility::log("Blank tile replacer initialized");
}

void BlankTileReplacer::render_objects() const {
  SDL_RenderCopy(Game::renderer(), bg_texture_.get(), nullptr, nullptr);
  std::ranges::for_each(blanks_tiles_, [](const auto &tile) static { tile.render(Game::renderer()); });
}

void BlankTileReplacer::handle_event(const SDL_Event& event) {
  switch(event.type) {
  case SDL_MOUSEMOTION:
    hovered_tile_ = Mouse::handle_hovering(blanks_tiles_);
    return;
  case SDL_MOUSEBUTTONUP:
    if (hovered_tile_ == nullptr) {
      return;
    }
    utility::log("Blank tile {} selected", hovered_tile_->letter);
    hovered_tile_->unhover();
    board_->put_on_board(*hovered_tile_);
    Mouse::set_default_cursor();
    hovered_tile_ = nullptr;
    Game::pop_until<Playing>();
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
  Game::push_game_state(this);
  hovered_tile_ = Mouse::handle_hovering(blanks_tiles_);
}

void BlankTileReplacer::replace_blank(Row_Col row_col, char letter) {
  assert(letter >= 'a' and letter <= 'z');
  board_->put_on_board(row_col, blanks_tiles_[static_cast<size_t>(letter - 'a')], false);
}
