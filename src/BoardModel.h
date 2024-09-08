#ifndef BOARDMODEL_H
#define BOARDMODEL_H

#include "constants.h"
#include "RowCol.h"
#include <SDL2/SDL_rect.h>
#include <array>
#include <cstdint>

class BoardModel final {

public:
  inline static constexpr std::array<Row_Col, constants::kTotalSquares> all_positions = std::invoke([]{
    std::array<Row_Col, constants::kTotalSquares> temp;
    // TODO: Replace with std::views::enumerate when clang supports it
    for(size_t index = 0; index < constants::kTotalSquares; ++index) {
      temp[index] = {static_cast<int8_t>(index/constants::kSquareNum),static_cast<int8_t>(index%constants::kSquareNum)};
    }
    return temp;
  });

  [[nodiscard]] static bool in_bounds(Row_Col row_col);

  BoardModel();

  void reset();

  [[nodiscard]] char get_letter(Row_Col row_col) const; 
  
  [[nodiscard]] std::uint8_t get_value(Row_Col row_col) const;

  void set_square(Row_Col row_col, char letter, std::uint8_t value, bool just_placed = true);

  void set_played(Row_Col row_col);
  
  void clear_square(Row_Col row_col);
  
  [[nodiscard]] bool is_empty(Row_Col row_col) const;

  [[nodiscard]] bool is_filled(Row_Col row_col) const;

  [[nodiscard]] bool is_already_played(Row_Col row_col) const;

  [[nodiscard]] bool has_adjacent_played_tile(Row_Col row_col) const;
  
  [[nodiscard]] int word_multiplier(Row_Col row_col) const; 
  
  [[nodiscard]] int letter_multiplier(Row_Col row_col) const;

  [[nodiscard]] bool has_already_played_tiles() const { return has_already_played_tiles_; }

private:
  static size_t get_index(Row_Col row_col);
  struct Square {
    char letter;
    std::uint8_t value;
    bool just_placed;
  };
  std::array<Square, constants::kTotalSquares> board_array_;
  bool has_already_played_tiles_;
};

#endif // BOARDMODEL_H
