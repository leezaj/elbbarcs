#ifndef PLAYING_H
#define PLAYING_H

#include "AssetPool.h"
#include "Background.h"
#include "Board.h"
#include "Button.h"
#include "ConfirmationDialog.h"
#include "Dawg.h"
#include "GameOver.h"
#include "GameState.h"
#include "Rack.h"
#include "Scoreboard.h"
#include "Solver.h"
#include "Tile.h"
#include "TileBag.h"
#include "TileSwapper.h"
#include "UnplayedTileCounter.h"
#include "WordOutliner.h"
#include "utility.h"

/**
 * @class Playing
 * @brief The playing state. This game state composes of all the game-related
 * features.
 *
 */
class Playing final : public GameState {
public:
  Playing(SDL_Renderer *rend, Mouse& mouse, const AssetPool &assets, GameStateManager& manager, ButtonMaker& button_maker);

  void handle_event(const SDL_Event &event) final;

  void render_objects() const final;

  [[nodiscard]] Texture get_snapshot(bool capture_rack = true);

  /** @brief Removes the tiles from the rack and swaps them with new tiles taken
   * from the bag, called from clicking 'Swap'
   *
   * @param positions A bitset where each 1 position indicates that the tile
   * should be swapped, and 0 otherwise.
   */
  void swap_tiles(const std::bitset<constants::kRackTileAmount> &positions);

  /**
   * @brief Check if a given placement on the board is valid, i.e. that it
   * follows the positional rules of Scrabble (placed tiles are all across or
   * down) as well as checking that there is a valid word in every position.
   */
  void evaluate_board();

  void restart_game();
private:
  void start_player_turn();

  void play_opponent_turn();

  void switch_turns();

  void recall_tiles();

  void play_turn();

  void put_best_move();

  void skip_turn();

  void click_button() const;

  void fill_player_rack();

  void fill_computer_rack();

  [[nodiscard]] Tile *take_tile();

  [[nodiscard]] const Button *button_at_pos() const;

  void handle_hovering();

  void show_shuffle_button();

  void show_recall_button();

  void ask_to_restart();

  Background background_;
  Dawg dictionary_;
  TileBag tile_bag_;
  BlankTileReplacer blank_replacer_;
  Board board_;
  Solver solver_;
  Rack rack_;
  std::vector<Tile> computer_tiles_;
  ConfirmationDialog confirm_dialog_;
  TileSwapper tile_swapper_;
  Button shuffle_, enter_, recall_, restart_, swap_, skip_, hint_, close_;
  std::vector<Button *> buttons_;
  Scoreboard scoreboard_;
  WordOutliner player_word_outliner_, computer_word_outliner_;
  Tile *selected_tile_{};
  SDL_Point tile_offset_{};
  const SelectableGameObject *hovered_{nullptr};
  std::optional<Solver::Solution> saved_best_move_;
  enum : std::uint8_t { RACK, BOARD } picked_up_from_{};
  bool mouse_down_{}, players_turn_{Random::coin_flip()}, display_player_points_{}, display_computer_points_{}, 
    player_has_valid_placement_{}, player_used_hints_{};
  std::uint8_t skipped_turns_in_a_row_{};
  GameOver game_over_;
  UnplayedTileCounter counter_;
};

#endif // PLAYING_H
