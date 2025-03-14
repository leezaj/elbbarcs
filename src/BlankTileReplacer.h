#ifndef BlankTileReplacer_H
#define BlankTileReplacer_H

#include "types.h"
#include "RowCol.h"
#include "Tile.h"

class Board;
class Playing;

/**
 * @class BlankTileReplacer
 * @brief A game state that is prompted whenever a blank tile is placed on the
 * board. The user is required to choose a letter to replace the blank tile.
 *
 */
class BlankTileReplacer final {
public:
  BlankTileReplacer(Board& board, Playing& playing_state);

  /**
   * @copydoc GameState::render_objects()
   */
  void render_objects() const;

  /**
   * @copydoc GameState::handle_event(const SDL_Event& event)
   */
  void handle_event(const SDL_Event& event);

  void replace_blank();

  /**
   * @brief Only to be used by the computer. The computer automatically knows
   * which letter it wants, so the replacer will do the same procedure as the
   * above function without any prompting.
   */
  void replace_blank(Row_Col row_col, char letter);
private:
  void load_tiles(SDL_Renderer* renderer);

  std::array<Texture, constants::kNumOfTiles-1> blanks_textures_;
  std::array<Tile, constants::kNumOfTiles-1> blanks_tiles_;
  Texture bg_texture_;
  Tile* hovered_tile_{};
  Board* board_{} ;
  Playing* playing_state_{};
};

#endif // BlankTileReplacer_H
