#ifndef PLAYING_H
#define PLAYING_H

#include "AssetPool.h"
#include "Board.h"
#include "Button.h"
#include "ConfirmationDialog.h"
#include "Dawg.h"
#include "GameOver.h"
#include "Rack.h"
#include "Scoreboard.h"
#include "Solver.h"
#include "Tile.h"
#include "TileBag.h"
#include "TileSwapper.h"
#include "UnplayedTileCounter.h"
#include "WordOutliner.h"

/**
 * @class Playing
 * @brief The playing state. This game state comprises of all the game-related features.
 */
class Playing final {
public:
  Playing(const AssetPool &assets, ButtonMaker& button_maker);

  void handle_event(const SDL_Event &event);

  void render_objects() const;

  [[nodiscard]] Texture get_snapshot(bool capture_rack = true);

  void swap_tiles(std::bitset<constants::kRackTileAmount> positions);

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

  void show_shuffle_button();

  void show_recall_button();

  void ask_to_restart();

  void handle_valid_placement(bool correct_words, Row_Col begin, Row_Col end, std::int32_t score);

  enum Buttons : std::uint8_t {
    SHUFFLE_OR_RECALL, ENTER, RESTART, SWAP, SKIP, HINT, CLOSE, NUM_OF_BUTTONS
  };

  SDL_Texture* background_;
  Dawg dictionary_;
  TileBag tile_bag_;
  BlankTileReplacer blank_replacer_;
  Board board_;
  Solver solver_;
  Rack rack_;
  std::vector<Tile> computer_tiles_;
  ConfirmationDialog confirm_dialog_;
  TileSwapper tile_swapper_;
  Button shuffle_, recall_;
  std::array<Button, NUM_OF_BUTTONS> buttons_; // -1 because shuffle or recall cannot appear at the same time
  Scoreboard scoreboard_;
  WordOutliner player_word_outliner_, computer_word_outliner_;
  Tile *selected_tile_{};
  SDL_Point tile_offset_{};
  std::variant<Button*, Tile*> hovered_;
  std::variant<std::monostate, Tile*, Button*> test_;
  std::optional<Solver::Solution> saved_best_move_;
  GameOver game_over_;
  UnplayedTileCounter counter_;
  std::uint8_t skipped_turns_in_a_row_{};
  enum : std::uint8_t { RACK, BOARD } picked_up_from_{};
  bool players_turn_{Random::coin_flip()}, player_has_valid_placement_{}, player_used_hints_{};
};

static_assert(GameState<Playing>);

#endif // PLAYING_H
