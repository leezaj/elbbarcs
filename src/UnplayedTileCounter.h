#ifndef UNPLAYEDTILECOUNTER_H
#define UNPLAYEDTILECOUNTER_H

#include "AssetPool.h"
#include "Text.h"
#include "Tile.h"
#include "constants.h"
#include "types.h"
#include <span>
class UnplayedTileCounter final {
public:
  UnplayedTileCounter(SDL_Renderer *renderer, const AssetPool &assets, std::span<const Tile> all_tiles);

  void render(SDL_Renderer* renderer) const;

  void update_count(SDL_Renderer* renderer, std::span<const Tile> recently_placed);

  void reset(SDL_Renderer* renderer);
private:
  UnplayedTileCounter(SDL_Renderer* renderer, TTF_Font* counter_font, std::array<SDL_Texture*, constants::kNumOfTiles> unique_textures);
  Texture all_tiles_, unavailable_shadows_;
  std::array<constants::TileInfo, constants::kNumOfTiles> infos_{constants::tile_info};
  std::array<Text, constants::kNumOfTiles> texts_;
};

#endif //UNPLAYEDTILECOUNTER_H
