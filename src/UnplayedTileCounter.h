#ifndef UNPLAYEDTILECOUNTER_H
#define UNPLAYEDTILECOUNTER_H

#include "AssetPool.h"
#include "Text.h"
#include "Tile.h"
#include "types.h"
#include <span>
#include <unordered_map>
class UnplayedTileCounter final {
public:
  UnplayedTileCounter(SDL_Renderer *renderer, const AssetPool &assets, std::span<const Tile> all_tiles);

  void render(SDL_Renderer* renderer) const;

  void update_count(SDL_Renderer* renderer, std::span<const Tile> played_tiles);

  void reset(SDL_Renderer* renderer);
private:
  Texture all_tiles_, blacked_out_tiles_;
  std::unordered_map<char, std::uint8_t> original_freqs_, current_freqs_;
  std::unordered_map<char, Text> counters_;
  std::size_t already_checked_idx_{};
};

#endif //UNPLAYEDTILECOUNTER_H
