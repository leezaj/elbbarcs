#ifndef TILESWAPPER_H
#define TILESWAPPER_H

#include "AssetPool.h"
#include "Button.h"
#include "GameState.h"
#include "Text.h"
#include "Tile.h"
#include "TileBag.h"
#include "constants.h"
#include <bitset>
#include <vector>

class Playing;

/**
 * @brief The game state that is responsible for swapping tiles. It is what gets
 * called when the player clicks the 'Swap' button.
 */
class TileSwapper final : public GameState {
public:
  TileSwapper(SDL_Renderer *rend, Mouse& mouse, GameStateManager& manager,
              Playing& playing_state, const AssetPool &assets, const TileBag& bag);

  void render_objects() const final;

  void handle_event(const SDL_Event& event) final;

  void ask(Texture background, std::vector<Tile> tiles);
private:
  void update_tiles_left_text();

  void update_selected_tiles_text(int count);

  std::bitset<constants::kRackTileAmount> selected_tiles_;
  Texture background_;
  std::vector<Tile> tiles_;
  std::array<Button, 2> buttons_;
  const Button* hovered_button_{};
  Tile* hovered_tile_{};
  GameStateManager* manager_;
  const TileBag* bag_;
  std::string selected_str_{"0 tiles selected"};
  Text remaining_text_, selected_text_;
  int tiles_left_{};
};

#endif // TILESWAPPER_H
