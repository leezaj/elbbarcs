#ifndef SOLVER_H
#define SOLVER_H

#include "BoardModel.h"
#include "Dawg.h"
#include "Tile.h"
#include <expected>
#include <unordered_set>
#include <variant>

/**
 * @class Solver
 * @brief A solver and move checker.
 *
 * @detail The backtracking algorithm used for the solver is outlined in the paper "The World's
 * Fastest Scrabble Program" by Andrew W. Appel and Guy J. Jacobson.
 */
class Solver final {
public:

  enum class InvalidPlacementError : std::uint8_t {
    NO_TILES_PROVIDED,
    MIDDLE_SQUARE_NOT_FILLED,
    NO_ADJACENT_TILE,
    NOT_STRAIGHT_LINE,
    NOT_CONTIGUOUS,
  };

  Solver(const BoardModel &board, const Dawg &dictionary);

  struct ValidPlacement {
    Row_Col begin{}, end{};
    std::int32_t score{};
  };

  struct ValidPlacementInvalidWords {
    ValidPlacement placement{};
    std::vector<std::string> invalid_words;
  };

  using Evaluation = std::expected<ValidPlacement, std::variant<InvalidPlacementError, ValidPlacementInvalidWords>>;

  struct Solution final {
    struct TileInfo final {
      Row_Col pos;
      char letter{};
      bool is_blank{};
    };
    std::vector<TileInfo> tiles;
    ValidPlacement info{};
  };

  [[nodiscard]] Evaluation get_board_evaluation(std::span<const Tile> placed_tiles) const;

  [[nodiscard]] Solution get_best_move(std::span<const Tile> rack);

private:

  static constexpr Row_Col kMiddleSquare = {.row = constants::kSquareNum / 2, .col = constants::kSquareNum / 2};

  enum Direction : std::uint8_t { ACROSS, DOWN };

  [[nodiscard]] std::vector<std::string> get_invalid_words(std::span<const Row_Col> positions, Direction dir) const;

  [[nodiscard]] std::int32_t calculate_score_human(const std::vector<Row_Col> &positions, Direction dir) const;

  static std::expected<std::span<const Tile>, InvalidPlacementError> ensure_nonempty(std::span<const Tile> input);

  std::expected<std::span<const Tile>, InvalidPlacementError> ensure_middle_squared_filled(std::span<const Tile> input) const;

  auto determine_direction(auto tile_positions) const  -> std::expected<std::pair<decltype(tile_positions), Direction>, InvalidPlacementError>;

  std::expected<std::tuple<Row_Col, Row_Col, Direction>, InvalidPlacementError> get_begin_and_end(auto tile_positions, Direction dir) const;

  Evaluation check_word_validity(Row_Col begin, Row_Col end, Direction dir) const;

  [[nodiscard]] bool is_contiguous(Row_Col begin, Row_Col end, Direction dir) const;


  struct CrossData final {
    std::uint32_t legal_letters;
    std::int32_t score;
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
  
  std::vector<Row_Col> expand_until_empty(Row_Col begin, Row_Col end, Direction dir) const;

  [[nodiscard]] std::int32_t calculate_score_computer(const std::vector<TileData> &played_tiles,
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
