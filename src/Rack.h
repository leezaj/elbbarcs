#ifndef RACK_H
#define RACK_H

#include "Tile.h"
#include "constants.h"
#include "types.h"
#include <span>
#include <vector>
/**
 * @class Rack
 * @brief A class that holds and manipulates tiles that a player can play.
 *
 */
class Rack final {
public:

  explicit Rack(SDL_Renderer* renderer);

  [[nodiscard]] const Tile* tile_at_pos(SDL_Point point) noexcept;

  /**
   * @brief Take a tile from the rack at the given point. The tile is released
   * from the rack.
   */
  [[nodiscard]] Tile* take_from(SDL_Point point);

  /**
   * @brief Take the first tile from the rack. The rack must not be empty.
   */
  [[nodiscard]] Tile take_tile();

  /**
   * @brief Take the ith tile from the rack. The tile must actually exist.
   */
  [[nodiscard]] Tile take_tile(std::uint32_t idx);

  /**
   * @brief Take the first tile with the specified letter. The tile must exist.
   */
  [[nodiscard]] Tile take_tile(char letter);

  [[nodiscard]] int missing_tiles() const;

  [[nodiscard]] size_t num_of_tiles() const;

  [[nodiscard]] bool empty() const { return num_of_tiles() == 0; }

  [[nodiscard]] std::vector<Tile> get_tiles() const { return {tiles_.begin(), tiles_.end()}; }

  [[nodiscard]] std::span<const Tile> tile_view() const {return tiles_;}

  /**
   * @brief Attempts to put a tile at a given point.
   */
  bool put(SDL_Point point, Tile tile);

  /**
   * @brief puts a tile in the first gap. Ensure the rack is missing a tile.
   */
  void put(Tile tile);

  void render(SDL_Renderer* renderer) const noexcept;

  /**
   * @brief Returns a tile taken with take_from() back to where it was.
   */
  void return_tile();

  void shuffle();

  /**
   * @brief Assumes that a tile taken from the rack, A, is at mouse_point. 
   * If mouse_point collides with an existing tile, B, then the
   * B will be swapped to where A was last taken from, and sets A's 'return
   * position' to be where B was..
   */
  void swap_tiles(SDL_Point mouse_point);

  /**
   * @brief Assumes that a tile from outside the rack, A, is at mouse_point. If
   * mouse_point collides with an existing tile, B, then B will
   * be moved to the first available gap to make room for A. The
   * caller must ensure that there is an empty gap for B.
   */
  void make_room_for_tile(SDL_Point mouse_point);

  private:
    static auto get_tile_idx(SDL_Point point);

    static constexpr SDL_Rect rect_{
        .x = constants::kRackTilePositions.front().x,
        .y = constants::kRackTilePositions.front().y,
        .w = constants::kRackTilePositions.back().x + constants::kTileWidth -
             constants::kRackTilePositions.front().x,
        .h = constants::kRackTilePositions.back().y - constants::kTileHeight};


    Tile create_gap_at(SDL_Point point);

    Tile* find_tile(SDL_Point point);

    std::array<Tile, constants::kRackTileAmount> tiles_;
    std::uint8_t taken_idx_{};
    Texture missing_tile_texture_;
    Tile taken_;
  };

#endif // RACK_H
