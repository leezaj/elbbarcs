#ifndef SOLVER_H
#define SOLVER_H

#include "BoardModel.h"
#include "Dawg.h"
#include "Tile.h"
#include <optional>
#include <unordered_set>

/**
 * @class Solver
 * @brief A solver and move checker.
 *
 * @detail The backtracking algorithm used for the solver is outlined in the paper "The World's
 * Fastest Scrabble Program" by Andrew W. Appel and Guy J. Jacobson.
 */
class Solver final {
public:

  Solver(const BoardModel &board, const Dawg &dictionary);

  struct BoardEvaluation final {
    bool is_valid_word{};
    std::int32_t total_score{};
    Row_Col word_begin_pos{};
    Row_Col word_end_pos{};
  };

  struct Solution {
    struct TileInfo final {
      Row_Col pos;
      char letter{};
      bool is_blank{};
    };
    std::vector<TileInfo> tiles;
    BoardEvaluation info{};
  };

  /**
   * @brief If the placed tiles constitute a legal Scrabble placement, then
   * returns information about the move. Otherwise, returns nothing.
   */
  [[nodiscard]] std::optional<BoardEvaluation>
  get_board_evaluation(const std::vector<Row_Col> &placed_tiles) const;

  /**
   * @brief Get the highest scoring move given a board state.
   */
  [[nodiscard]] Solution get_best_move(const std::vector<Tile> &rack);

private:
  /* MOVE VALIDATION FUNCTIONS */
  enum Direction : std::uint8_t { ACROSS, DOWN };

  /**
   * @brief Determines the direction of a move, or returns an optional if the move has no direction
   */
  [[nodiscard]] std::optional<Direction> determine_direction(const auto &placed_tiles) const;

  /**
   * @brief Determines whether the placed tiles are contiguous (no gaps between them)
   */
  [[nodiscard]] bool is_contiguous(Row_Col begin, Row_Col end, Direction dir) const;

  /**
   * @brief Determines whether the word and all cross-words made by the tiles constitute valid words
   */
  [[nodiscard]] bool words_valid(const std::vector<Row_Col> &positions, Direction dir) const;

  [[nodiscard]] std::int32_t calculate_score_human(const std::vector<Row_Col> &positions, Direction dir) const;

  /* BACKTRACKING ALGORITHM FUNCTIONS */

  struct CrossData final {
    std::uint32_t legal_letters;
    std::int16_t score;
    bool has_connecting_tiles;
  };

  struct TileData final {
    char letter;
    std::uint8_t value;
  };


  static Row_Col move(Row_Col row_col, int row_delta, int col_delta);

  static Row_Col before(Row_Col row_col, Direction dir);

  static Row_Col after(Row_Col row_col, Direction dir);

  static Row_Col before_cross(Row_Col row_col, Direction dir);

  static Row_Col after_cross(Row_Col row_col, Direction dir);

  void get_all_anchors();

  [[nodiscard]] std::int32_t
  calculate_score_computer(const std::vector<TileData> &played_tiles,
                           Row_Col last_pos, Direction dir) const;

  void handle_legal_move(const std::vector<TileData> &played_tiles, Row_Col last_pos, Direction dir);

  void before_part(std::vector<TileData> tiles_before, Direction dir, Row_Col anchor_pos, 
                   std::vector<TileData> unplayed_tiles, const Dawg::Node *current_node, int expand_limit);

  void extend_after(std::vector<TileData> tiles_before, Direction dir, Row_Col next_pos, 
                    std::vector<TileData> unplayed_tiles, const Dawg::Node *current_node, bool anchor_filled);

  void make_cross_checks(Direction dir);

  std::unordered_set<Row_Col> current_anchors_;
  Solution solution_{};
  std::unordered_map<Row_Col, CrossData> cross_checks_;
  const BoardModel *board_;
  const Dawg *dict_;
};

#endif // SOLVER_H
