#ifndef TILEBAG_H
#define TILEBAG_H

#include "Tile.h"
#include "constants.h"
#include "types.h"
#include <span>

class TileBag final {
public:

explicit TileBag(SDL_Renderer *renderer);

[[nodiscard]] size_t tiles_left() const;

[[nodiscard]] bool empty() const;

[[nodiscard]] std::span<const Tile> tiles_view() const;

[[nodiscard]] Tile take_from();

[[nodiscard]] std::vector<Tile> swap(std::span<const Tile> with);

void shuffle();

void reset();

private:
  void load_tiles(SDL_Renderer* renderer);
  std::array<Texture, constants::kBagTileAmount> tile_textures_;
  std::array<Tile, constants::kBagTileAmount> tile_bag_;
  std::size_t take_from_index_ = 0;
};

#endif // TILEBAG_H
