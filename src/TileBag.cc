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
#include <vector>

namespace {
struct TileInfo {
  b::EmbedInternal::EmbeddedFile file;
  char letter;
  std::uint8_t frequency;
  std::uint8_t value;
};
} //namespace


void TileBag::load_tiles(SDL_Renderer* renderer) {
  std::array<TileInfo, constants::kNumOfTiles> tile_infos{{
      {b::embed<"assets/tiles/a.png">(), 'a', 9, 1},
      {b::embed<"assets/tiles/b.png">(), 'b', 2, 3},
      {b::embed<"assets/tiles/blank_tile.png">(), constants::kTileBlankChar, 2, 0}, 
      {b::embed<"assets/tiles/c.png">(),'c', 2, 3},
      {b::embed<"assets/tiles/d.png">(),'d', 4, 2},
      {b::embed<"assets/tiles/e.png">(),'e', 12, 1},
      {b::embed<"assets/tiles/f.png">(),'f', 2, 4},
      {b::embed<"assets/tiles/g.png">(),'g', 3, 2},
      {b::embed<"assets/tiles/h.png">(),'h', 2, 4},
      {b::embed<"assets/tiles/i.png">(),'i', 9, 1},
      {b::embed<"assets/tiles/j.png">(),'j', 1, 8},
      {b::embed<"assets/tiles/k.png">(),'k', 1, 5},
      {b::embed<"assets/tiles/l.png">(),'l', 4, 1},
      {b::embed<"assets/tiles/m.png">(),'m', 2, 3},
      {b::embed<"assets/tiles/n.png">(),'n', 6, 1},
      {b::embed<"assets/tiles/o.png">(),'o', 8, 1},
      {b::embed<"assets/tiles/p.png">(),'p', 2, 3},
      {b::embed<"assets/tiles/q.png">(),'q', 1, 10},
      {b::embed<"assets/tiles/r.png">(),'r', 6, 1},
      {b::embed<"assets/tiles/s.png">(),'s', 4, 1},
      {b::embed<"assets/tiles/t.png">(),'t', 6, 1},
      {b::embed<"assets/tiles/u.png">(),'u', 4, 1},
      {b::embed<"assets/tiles/v.png">(),'v', 2, 4},
      {b::embed<"assets/tiles/w.png">(),'w', 2, 4},
      {b::embed<"assets/tiles/x.png">(),'x', 1, 8},
      {b::embed<"assets/tiles/y.png">(),'y', 2, 4},
      {b::embed<"assets/tiles/z.png">(),'z', 1, 10},
  }};
  std::vector<Texture> textures;
  textures.reserve(constants::kBagTileAmount);
  std::vector<Tile> tiles;
  tiles.reserve(constants::kBagTileAmount);
  for(const auto& tile: tile_infos) {
    RWops buffer {SDL_RWFromConstMem(tile.file.data(), static_cast<int>(tile.file.size()))};
    Surface temp{IMG_Load_RW(buffer.get(), 0)};
    for (auto i = 0; i < tile.frequency; ++i) {
      textures.emplace_back(SDL_CreateTextureFromSurface(renderer, temp.get()));
      tiles.emplace_back(textures.back().get(), SDL_Rect{.x = 0,
                                                         .y = 0,
                                                         .w = constants::kTileWidth,
                                                         .h = constants::kTileHeight}, tile.letter, tile.value);
    }
  }
  tile_textures_ = std::move(textures);
  tile_bag_ = std::move(tiles);
}

TileBag::TileBag(SDL_Renderer* renderer)
{
  load_tiles(renderer);
}

[[nodiscard]] Tile TileBag::take_from() {
  Tile back = tile_bag_.back();
  tile_bag_.pop_back();
  return back;
}

[[nodiscard]] size_t TileBag::tiles_left() const {
  return tile_bag_.size();
}

[[nodiscard]] bool TileBag::empty() const {return tile_bag_.empty();}

void TileBag::shuffle() {std::ranges::shuffle(tile_bag_, Random::engine);}

void TileBag::put_tiles(std::vector<Tile> tiles) {
  tile_bag_.insert(tile_bag_.end(), tiles.begin(), tiles.end());
}
