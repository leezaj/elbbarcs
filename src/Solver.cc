#include "Solver.h"
#include "utility.h"
#include <algorithm>
#include <numeric>
#include <ranges>


Solver::Solver(const BoardModel& board, const Dawg& dict):
  board_{&board},
  dict_{&dict}
{
  solution_.tiles.reserve(constants::kRackTileAmount);
  utility::log("Solver initialized");
}

Row_Col Solver::move(Row_Col row_col, int row_delta, int col_delta) {
  return {.row = static_cast<int8_t>(row_col.row + row_delta), .col = static_cast<int8_t>(row_col.col + col_delta)};
}

Row_Col Solver::before(Row_Col row_col, Direction dir) {
  if (dir == ACROSS) {
    return move(row_col, 0, -1);
  }
  return move(row_col, -1, 0);
}

Row_Col Solver::after(Row_Col row_col, Direction dir) {
  if (dir == ACROSS) {
    return move(row_col, 0, 1);
  }
  return move(row_col, 1, 0);
}

Row_Col Solver::before_cross(Row_Col row_col, Direction dir) {
  if (dir == ACROSS) {
    return move(row_col, -1, 0);
  }
  return move(row_col, 0, -1);
}

Row_Col Solver::after_cross(Row_Col row_col, Direction dir) {
  if (dir == ACROSS) {
    return move(row_col, 1, 0);
  }
  return move(row_col, 0, 1);
}

bool Solver::is_contiguous(Row_Col begin, Row_Col end, Direction dir) const {
  while (board_->is_filled(begin)) {
    if (begin == end) {
      return true;
    }
    begin = after(begin, dir);
  }
  return false;
}

std::vector<std::string> Solver::get_invalid_words(std::span<const Row_Col> positions, Direction dir) const {
  std::vector<std::string> result;
  auto word = positions | std::views::transform(std::bind_front(&BoardModel::get_letter, board_));
  // ensure the actual word is valid
  if (not dict_->word_exists(word)) {
    result.emplace_back(word.begin(), word.end());
  }
  // ensure all cross words are valid
  std::string cross_word;
  for(auto pos: positions) {
    cross_word.clear();
    if (board_->is_already_played(pos)) {
      continue;
    }
    auto scan_before = pos, scan_after = pos;
    std::size_t size = 0;
    while(board_->is_filled(before_cross(scan_before, dir))) {
      ++size;
      scan_before = before_cross(scan_before, dir);
    }
    while(board_->is_filled(after_cross(scan_after, dir))) {
      ++size;
      scan_after = after_cross(scan_after, dir);
    }
    if (size == 0) {
      continue;
    }
    cross_word.reserve(++size);
    for(auto i = 0UZ; i<size; ++i) {
      cross_word.push_back(board_->get_letter(scan_before));
      scan_before = after_cross(scan_before,dir);
    }
    if (not dict_->word_exists(cross_word)) {
      result.push_back(std::move(cross_word));
    }
  }
  return result;
}

std::int32_t Solver::calculate_score_human(const std::vector<Row_Col> &positions, Direction dir) const {
  // lambda for calculating cross scores
  const auto cross_score = [dir, this](Row_Col pos) -> std::int32_t {
    std::int32_t score = 0;
    auto scan_before = pos, scan_after = pos;
    while(board_->is_filled(before_cross(scan_before, dir))) {
      scan_before = before_cross(scan_before, dir);
      score += board_->get_value(scan_before);
    }
    while(board_->is_filled(after_cross(scan_after, dir))) {
      scan_after = after_cross(scan_after, dir);
      score += board_->get_value(scan_after);
    }
    if(scan_before != scan_after) {
      score += board_->get_value(pos) * board_->letter_multiplier(pos);
      score *= board_->word_multiplier(pos);
    }
    return score;
  };
  // end lambda
  const auto transform_positions_with = [&positions, this](auto func) {
    return positions | std::views::transform(std::bind_front(func, board_));
  };
  const std::int32_t word_val = std::invoke([&]{
    const auto vals = transform_positions_with(&BoardModel::get_value);
    const auto mults = transform_positions_with(&BoardModel::letter_multiplier);
    return std::transform_reduce(vals.begin(), vals.end(), mults.begin(), std::int32_t{});
  });
  const std::int32_t word_mult = std::invoke([&]{
    const auto mults = transform_positions_with(&BoardModel::word_multiplier);
    return std::reduce(mults.begin(), mults.end(), std::int32_t{1}, std::multiplies{});
  });
  std::int32_t total_word =  word_val * word_mult;
  std::int32_t actually_placed = 0;
  std::int32_t total_cross = 0;
  for(auto pos: positions) {
    if(not board_->is_already_played(pos)){
      total_cross += cross_score(pos);
      ++actually_placed;
    }
  }
  if (actually_placed == constants::kRackTileAmount) {
    total_word += constants::kBingoScoreValue;
  }
  return total_word + total_cross;
}

std::int32_t Solver::calculate_score_computer(const std::vector<TileData> &played_tiles, Row_Col last_pos, Direction dir) const {
  std::int32_t total_cross = 0, total_word = 0, total_mult = 1, actually_placed = 0;
  auto vals = played_tiles | std::views::transform(&TileData::value) | std::views::reverse;
  for(auto val : vals) {
    std::int32_t this_letter = val*board_->letter_multiplier(last_pos);
    std::int32_t this_mult = board_->word_multiplier(last_pos);
    total_word += this_letter;
    total_mult *= this_mult;
    if(not board_->is_already_played(last_pos)) {
      ++actually_placed;
      if (auto it = cross_checks_.at(last_pos); it.has_connecting_tiles) {
        std::int32_t cross_score = it.score;
        cross_score += this_letter;
        cross_score *= this_mult;
        total_cross += cross_score;
      }
    }
    last_pos = before(last_pos, dir);
  }
  total_word *= total_mult;
  if (actually_placed == constants::kRackTileAmount) {
    total_word += constants::kBingoScoreValue;
  }
  return total_word + total_cross;
}

auto Solver::determine_direction(auto tile_positions) const  -> std::expected<std::pair<decltype(tile_positions), Direction>, InvalidPlacementError> {
  assert(!tile_positions.empty());
  const auto& first = tile_positions.front();
  if (tile_positions.size() == 1) {
    if (board_->is_filled(before(first, ACROSS)) or board_->is_filled(after(first, ACROSS))) {
      return std::make_pair(tile_positions, ACROSS);
    }
    if (board_->is_filled(before(first, DOWN)) or board_->is_filled(after(first, DOWN))) {
      return std::make_pair(tile_positions, DOWN);
    }
    return std::unexpected{InvalidPlacementError::NO_ADJACENT_TILE};
  }
  if (std::ranges::all_of(tile_positions, std::bind_front(std::equal_to{}, first.row), &Row_Col::row)) {
    return std::make_pair(tile_positions, ACROSS);
  }
  if (std::ranges::all_of(tile_positions, std::bind_front(std::equal_to{}, first.col), &Row_Col::col)) {
    return std::make_pair(tile_positions, DOWN);
  }
  return std::unexpected{InvalidPlacementError::NOT_STRAIGHT_LINE};
}

std::vector<Row_Col> Solver::expand_until_empty(Row_Col begin, Row_Col end, Direction dir) const {
  std::vector<Row_Col> positions;
  while (board_->is_filled(before(begin, dir))) {
    begin = before(begin, dir);
  }
  while (board_->is_filled(after(end, dir))) {
    end = after(end, dir);
  }
  positions.reserve(static_cast<size_t>(dir == ACROSS ? end.col - begin.col + 1 : end.row - begin.row + 1));
  for(Row_Col start = begin, stop = after(end,dir); start != stop; start = after(start, dir)) {
    positions.push_back(start);
  }
  return positions;
}

std::expected<std::span<const Tile>, Solver::InvalidPlacementError> Solver::ensure_nonempty(std::span<const Tile> input) {
  if(input.empty()) {
    return std::unexpected{InvalidPlacementError::NO_TILES_PROVIDED};
  }
  return input;
}

std::expected<std::span<const Tile>, Solver::InvalidPlacementError> Solver::ensure_middle_squared_filled(std::span<const Tile> input) const {
  if(not board_->is_filled(kMiddleSquare)) {
    return std::unexpected{InvalidPlacementError::MIDDLE_SQUARE_NOT_FILLED};
  }
  return input;
}

namespace {
auto get_positions_of_tiles(std::span<const Tile> input) {
  return input | std::views::transform([](const Tile& tile) static { return to_row_col(tile.point());});
}
} // namespace

std::expected<std::tuple<Row_Col, Row_Col, Solver::Direction>, Solver::InvalidPlacementError> Solver::get_begin_and_end(auto tile_positions, Direction dir) const {
  auto [begin, end] = std::ranges::minmax(tile_positions, {}, dir == ACROSS ? &Row_Col::col : &Row_Col::row);
  if(not is_contiguous(begin, end, dir)) {
    return std::unexpected{InvalidPlacementError::NOT_CONTIGUOUS};
  }
  if(board_->has_already_played_tiles() and std::ranges::none_of(tile_positions, std::bind_front(&BoardModel::has_adjacent_played_tile, board_))) {
      return std::unexpected{InvalidPlacementError::NO_ADJACENT_TILE};
  }
  return std::tuple{begin, end, dir};
}

Solver::Evaluation Solver::check_word_validity(Row_Col begin, Row_Col end, Direction dir) const {
  std::vector<Row_Col> expand = expand_until_empty(begin, end, dir);
  assert(!expand.empty());
  ValidPlacement placement{.begin = expand.front(), .end = expand.back(), .score = calculate_score_human(expand, dir)};
  if(auto invalid_words = get_invalid_words(expand, dir); !invalid_words.empty()) {
    return std::unexpected<Evaluation::error_type>{std::in_place, std::in_place_type<ValidPlacementInvalidWords>, placement, std::move(invalid_words) };
  }
  return placement;
}

Solver::Evaluation Solver::get_board_evaluation(std::span <const Tile> placed_tiles) const {
  return  ensure_nonempty(placed_tiles)
          .and_then(std::bind_front(&Solver::ensure_middle_squared_filled, this))
          .transform(get_positions_of_tiles)
          .and_then([this](auto tile_positions){return determine_direction(tile_positions);})
          .and_then([this](auto pos_and_dir) { return get_begin_and_end(pos_and_dir.first, pos_and_dir.second);})
          .transform_error([](InvalidPlacementError error) static { return Evaluation::error_type(error); })
          .and_then([this](auto tuple) {return std::apply(std::bind_front(&Solver::check_word_validity, this), tuple);}); 
}

void Solver::get_all_anchors() {
  auto all_anchors = BoardModel::all_positions | std::views::filter([this](Row_Col rc) {
      return board_->is_empty(rc) and board_->has_adjacent_played_tile(rc);
  });
  current_anchors_.insert(all_anchors.begin(), all_anchors.end());
}

void Solver::handle_legal_move(const std::vector<TileData> &played_tiles, Row_Col last_pos, Direction dir){
  auto score = calculate_score_computer(played_tiles, last_pos, dir);
  if(score > solution_.info.score) {
    solution_.tiles.clear();
    solution_.info.score = score;
    solution_.info.end = last_pos;
    for(auto [letter, value]: played_tiles | std::views::reverse) {
      if (not board_->is_already_played(last_pos)) {
        solution_.tiles.emplace_back(last_pos, letter, value == 0);
      }
      last_pos = before(last_pos, dir);
    }
    solution_.info.begin = after(last_pos, dir);
  }
}

void Solver::before_part(std::vector<TileData> tiles_before, Direction dir, Row_Col anchor_pos, 
                         std::vector<TileData> unplayed_tiles, const Dawg::Node *current_node, int expand_limit) {
  extend_after(tiles_before, dir, anchor_pos, unplayed_tiles, current_node, false);
  if (expand_limit <= 0) {
    return;
  }
  for (auto [letter, index] : current_node->edges) {
    // if we have the letter in our rack, put it
    if (auto it = std::ranges::find(unplayed_tiles, letter, &TileData::letter); it != unplayed_tiles.end()) {
      auto [to_play, value] = *it;
      std::iter_swap(unplayed_tiles.rbegin(), it); unplayed_tiles.pop_back();
      tiles_before.emplace_back(to_play, value);
      before_part(tiles_before, dir, anchor_pos, unplayed_tiles, dict_->node_at_index(index), expand_limit - 1);
      tiles_before.pop_back();
      unplayed_tiles.emplace_back(to_play, value);
    }
    // try a blank for every letter in the current node's edges if we have one
    if (auto it = std::ranges::find(unplayed_tiles, constants::kTileBlankChar, &TileData::letter); it != unplayed_tiles.end()) {
      std::iter_swap(unplayed_tiles.rbegin(), it); unplayed_tiles.pop_back();
      tiles_before.emplace_back(letter, 0);
      before_part(tiles_before, dir, anchor_pos, unplayed_tiles, dict_->node_at_index(index), expand_limit - 1);
      tiles_before.pop_back();
      unplayed_tiles.emplace_back(constants::kTileBlankChar, 0);
    }
  }
}

void Solver::extend_after(std::vector<TileData> tiles_before, Direction dir, Row_Col next_pos, 
                          std::vector<TileData> unplayed_tiles, const Dawg::Node *current_node, bool anchor_filled) {
  if(not board_->is_filled(next_pos) and current_node->final and anchor_filled){
    handle_legal_move(tiles_before, before(next_pos, dir), dir);
  }
  if (not BoardModel::in_bounds(next_pos)) {
    return;
  }
  // if there's a letter where we want to play, put the existing letter and continue
  if(board_->is_filled(next_pos)) {
    char letter = board_->get_letter(next_pos);
    std::uint8_t val = board_->get_value(next_pos);
    if (const auto *it = dict_->search_edges_for(current_node, letter); it != nullptr){
      tiles_before.emplace_back(letter, val);
      extend_after(std::move(tiles_before), dir, after(next_pos, dir), std::move(unplayed_tiles), it, true);
    }
  } else {
    // if there's no letter, decide what to play based on the node's edges
    for (auto [letter, index] : current_node->edges) {
      // if we've calculated that the letter isn't legal here with cross checks, skip this letter
      std::uint32_t legal_here = cross_checks_.at(next_pos).legal_letters;
      if ((legal_here&(1U << static_cast<std::uint32_t>(letter - 'a'))) == 0) {
        continue;
      }
      // if we have the letter in the node's edge in our rack, put it and backtrack
      if (auto it = std::ranges::find(unplayed_tiles,letter,&TileData::letter); it != unplayed_tiles.end()) {
        auto [rack_letter, rack_value] = *it;
        std::iter_swap(unplayed_tiles.rbegin(), it); unplayed_tiles.pop_back();
        tiles_before.emplace_back(rack_letter, rack_value);
        extend_after(tiles_before, dir, after(next_pos, dir), unplayed_tiles, dict_->node_at_index(index), true);
        tiles_before.pop_back();
        unplayed_tiles.emplace_back(rack_letter, rack_value);
      }
      // if we have a blank, try every letter in the node's edges and backtrack
      if (auto it = std::ranges::find(unplayed_tiles, constants::kTileBlankChar, &TileData::letter); it != unplayed_tiles.end()) {
        std::iter_swap(unplayed_tiles.rbegin(), it); unplayed_tiles.pop_back();
        tiles_before.emplace_back(letter, 0);
        extend_after(tiles_before, dir, after(next_pos, dir), unplayed_tiles, dict_->node_at_index(index), true);
        tiles_before.pop_back();
        unplayed_tiles.emplace_back(constants::kTileBlankChar, 0);
      }
    }
  }
}

void Solver::make_cross_checks(Direction dir) {
  static constexpr std::uint32_t all_letters =  0b11111111111111111111111111;
  cross_checks_.clear();
  static thread_local std::string split_word;
  for(auto empty : BoardModel::all_positions | std::views::filter([this](Row_Col rc) {return board_->is_empty(rc);})) {
    Row_Col up_scan = empty, down_scan = empty;
    std::size_t up_size = 0, down_size = 0;
    std::uint32_t legal_here{};
    std::int32_t cross_score{};
    bool is_connected{false};
    while(board_->is_filled(before_cross(up_scan, dir))){
      up_scan = before_cross(up_scan, dir);
      ++up_size;
    }
    while(board_->is_filled(after_cross(down_scan, dir))) {
      down_scan = after_cross(down_scan, dir);
      ++down_size;
    }
    if(up_size == 0 and down_size == 0) {
      legal_here = all_letters;
    } else {
      split_word.clear();
      is_connected = true;
      for(auto i = 0UZ; i<up_size; ++i) {
        split_word += board_->get_letter(up_scan);
        cross_score += board_->get_value(up_scan);
        up_scan = after_cross(up_scan, dir);
      }
      std::size_t separating_idx = split_word.size();
      split_word += '\0';
      down_scan = after_cross(empty, dir);
      for(auto i = 0UZ; i<down_size; ++i) {
        split_word += board_->get_letter(down_scan);
        cross_score += board_->get_value(down_scan);
        down_scan = after_cross(down_scan, dir);
      }
      for(char ch = 'a'; ch <='z'; ++ch) {
        split_word[separating_idx] = ch;
        if (dict_->word_exists(split_word)) {
          legal_here |= (1U << static_cast<std::uint32_t>(ch - 'a'));
        }
      }
    }
    cross_checks_.try_emplace(empty, legal_here, cross_score, is_connected);
  }
}

Solver::Solution Solver::get_best_move(std::span<const Tile> rack) {
  static constexpr std::array<Direction, 2> all_directions{ACROSS,DOWN};
  const std::vector<TileData> tiles = utility::map(rack, [](const Tile& tile) static { 
    return TileData{.letter = tile.letter(), .value = tile.value()}; 
  });
  solution_.info = {};
  solution_.tiles.clear();
  current_anchors_.clear();
  std::span<const Direction> directions;
  if (board_->has_already_played_tiles()) {
    directions = all_directions;
    get_all_anchors();
  } else {
    directions = std::views::counted(all_directions.begin(), 1);
    current_anchors_.emplace(kMiddleSquare);
  }
  for (auto dir : directions) {
    make_cross_checks(dir);
    for (auto anchor : current_anchors_) {
      auto scan_pos = anchor;
      std::uint8_t num_of_tiles_before_anchor{};
      while (board_->is_filled(before(scan_pos, dir))) {
        ++num_of_tiles_before_anchor;
        scan_pos = before(scan_pos, dir);
      }
      if (num_of_tiles_before_anchor == 0) {
        scan_pos = before(scan_pos, dir);
        std::int32_t expand_limit = 0;
        while (board_->is_empty(scan_pos) and not current_anchors_.contains(scan_pos)) {
          ++expand_limit;
          scan_pos = before(scan_pos, dir);
        }
        before_part(std::vector<TileData>{}, dir, anchor, tiles, dict_->root(), expand_limit);
      } else {
        std::vector<TileData> tiles_before;
        tiles_before.reserve(num_of_tiles_before_anchor);
        for (auto i = 0; i < num_of_tiles_before_anchor; ++i) {
          tiles_before.emplace_back(board_->get_letter(scan_pos), board_->get_value(scan_pos));
          scan_pos = after(scan_pos, dir);
        }
        auto whats_there = tiles_before | std::views::transform(&TileData::letter);
        if(const auto *node = dict_->lookup_prefix(whats_there); node != nullptr) {
          extend_after(std::move(tiles_before), dir, anchor, tiles, node, false);
        }
      }
    }
  }
  return solution_;
}
