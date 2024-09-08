#include "AssetPool.h"
#include "Tile.h"
#include "TileBag.h"
#include "constants.h"
#include "utility.h"
#include <SDL2/SDL_render.h>
#include <algorithm>
#include <cassert>
#include <print>
#include <vector>

namespace {

const auto tiles_path = AssetPool::assets_path() / "tiles";

struct TileInfo {
  char letter;
  std::uint8_t frequency;
  std::uint8_t value;
};

constexpr std::array<TileInfo, constants::kNumOfTiles> kTileVals{{
    {constants::kTileBlankChar, 2, 0}, {'a', 9, 1}, {'b', 2, 3},
    {'c', 2, 3}, {'d', 4, 2}, {'e', 12, 1},
    {'f', 2, 4}, {'g', 3, 2}, {'h', 2, 4},
    {'i', 9, 1}, {'j', 1, 8}, {'k', 1, 5},
    {'l', 4, 1}, {'m', 2, 3}, {'n', 6, 1},
    {'o', 8, 1}, {'p', 2, 3}, {'q', 1, 10},
    {'r', 6, 1}, {'s', 4, 1}, {'t', 6, 1},
    {'u', 4, 1}, {'v', 2, 4}, {'w', 2, 4},
    {'x', 1, 8}, {'y', 2, 4}, {'z', 1, 10},
}};

} //namespace


void TileBag::load_tiles(SDL_Renderer* renderer) {
  std::vector<Texture> textures;
  textures.reserve(constants::kBagTileAmount);
  std::vector<Tile> tiles;
  tiles.reserve(constants::kBagTileAmount);
  for(const auto& file: std::filesystem::directory_iterator(tiles_path)) {
    std::string_view filename{file.path().stem().c_str()};
    // if it's a blank tile, the filename will be something like 'blank_tile.png'. otherwise it will be one letter only
    const auto *const val = std::ranges::find(kTileVals, filename.size() == 1 ? filename.front() : constants::kTileBlankChar, &TileInfo::letter);
    assert(val != kTileVals.end());
    Surface temp{IMG_Load(file.path().c_str())};
    for (auto i = 0; i < val->frequency; ++i) {
      textures.emplace_back(SDL_CreateTextureFromSurface(renderer, temp.get()));
      tiles.emplace_back(textures.back().get(), SDL_Rect{.x = 0,
                                                         .y = 0,
                                                         .w = constants::kTileWidth,
                                                         .h = constants::kTileHeight}, val->letter, val->value);
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
