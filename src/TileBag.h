#ifndef TILEBAG_H
#define TILEBAG_H

#include "Tile.h"
#include "types.h"
#include <span>
#include <vector>

class TileBag final {
public:

explicit TileBag(SDL_Renderer *renderer);

[[nodiscard]] Tile take_from();

[[nodiscard]] size_t tiles_left() const;

[[nodiscard]] bool empty() const;

void shuffle();

void put_tiles(std::vector<Tile> tiles);

[[nodiscard]] std::span<const Tile> tiles_view() const { return tile_bag_; }
private:
  void load_tiles(SDL_Renderer* renderer);
  std::vector<Texture> tile_textures_;
  std::vector<Tile> tile_bag_;
};

#endif // TILEBAG_H
