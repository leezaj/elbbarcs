#include "BoardModel.h"
#include <cassert>
#include <ranges>

BoardModel::BoardModel() {
  reset();
}

void BoardModel::reset() {
  board_array_.fill({constants::kTileGapChar, 0, true});
  has_already_played_tiles_ = false;
}

char BoardModel::get_letter(Row_Col row_col) const {
  return board_array_[get_index(row_col)].letter;
}

std::uint8_t BoardModel::get_value(Row_Col row_col) const {
  return board_array_[get_index(row_col)].value;
}

void BoardModel::set_square(Row_Col row_col, char letter, std::uint8_t value, bool just_placed) {
  board_array_[get_index(row_col)] = {letter, value, just_placed};
}

void BoardModel::set_played(Row_Col row_col) {
  board_array_[get_index(row_col)].just_placed = false;
  has_already_played_tiles_ = true;
}

void BoardModel::clear_square(Row_Col row_col) {
  set_square(row_col, constants::kTileGapChar, 0, true);
}

bool BoardModel::in_bounds(Row_Col row_col){
  auto [row, col] = row_col;
  static constexpr auto dim = constants::kSquareNum;
  return row >= 0 and row < dim and col >= 0 and col < dim;
}

bool BoardModel::is_empty(Row_Col row_col) const {
  return in_bounds(row_col) and get_letter(row_col) == constants::kTileGapChar;
}

bool BoardModel::is_filled(Row_Col row_col) const {
  return in_bounds(row_col) and get_letter(row_col) != constants::kTileGapChar;
} 

bool BoardModel::is_already_played(Row_Col row_col) const {
  if (not in_bounds(row_col)) {
    return false;
  }
  auto square = board_array_[get_index(row_col)];
  return square.letter != constants::kTileGapChar and not square.just_placed;
}

bool BoardModel::has_adjacent_played_tile(Row_Col row_col) const {
  auto [row, col] = row_col;
  return is_already_played({static_cast<int8_t>(row - 1), col}) or
         is_already_played({static_cast<int8_t>(row + 1), col}) or
         is_already_played({row, static_cast<int8_t>(col - 1)}) or 
         is_already_played({row, static_cast<int8_t>(col + 1)});
}

int BoardModel::word_multiplier(Row_Col row_col) const {
  if (is_already_played(row_col)) {
    return 1;
  }
  switch(get_index(row_col)) {
    case 0:
    case 7:
    case 14:
    case 105:
    case 119:
    case 210:
    case 217:
    case 224:
      return 3;
    case 16:
    case 28:
    case 32:
    case 42:
    case 48:
    case 56:
    case 64:
    case 70:
    case 112:
    case 154:
    case 160:
    case 168:
    case 176:
    case 182:
    case 192:
    case 196:
    case 208:
      return 2;
    default:
      return 1;
  }
}  

int BoardModel::letter_multiplier(Row_Col row_col) const {
  if (is_already_played(row_col)) {
    return 1;
  }
    switch(get_index(row_col)) {
      case 20:
      case 24:
      case 76:
      case 80:
      case 84:
      case 88:
      case 136:
      case 140:
      case 144:
      case 148:
      case 200:
      case 204:
        return 3;
      case 3:
      case 11:
      case 36:
      case 38:
      case 45:
      case 52:
      case 59:
      case 92:
      case 96:
      case 98:
      case 102:
      case 108:
      case 116:
      case 122:
      case 126:
      case 128:
      case 132:
      case 165:
      case 172:
      case 179:
      case 186:
      case 188:
      case 213:
      case 221:
        return 2;
      default:
        return 1;
    }
  }
size_t BoardModel::get_index(Row_Col row_col) {
  auto [row, col] = row_col;
  return static_cast<size_t>(col) + constants::kSquareNum * static_cast<size_t>(row);
}
