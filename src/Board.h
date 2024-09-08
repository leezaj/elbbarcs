#ifndef BOARD_H
#define BOARD_H

#include "BoardModel.h"
#include "BlankTileReplacer.h"
#include "Tile.h"
#include <optional>
#include <span>
#include <vector>

class Board final {

public:
  Board(SDL_Renderer *renderer, BlankTileReplacer& blank_replacer);

  void reset();

  bool put_on_board(const Tile &tile);

  bool put_on_board(Row_Col pos, Tile tile, bool is_player);

  void return_tile();

  [[nodiscard]] const Tile* tile_at_pos(SDL_Point point) const;

  [[nodiscard]] Tile* take_from_board(SDL_Point point);

  [[nodiscard]] Tile take_oldest_placed();

  [[nodiscard]] Tile take_tile();

  [[nodiscard]] bool empty() const;

  [[nodiscard]] size_t num_of_tiles() const;

  [[nodiscard]] bool has_recently_placed_tiles() const;

  [[nodiscard]] std::vector<Row_Col> placed_tile_positions() const;

  [[nodiscard]] const BoardModel& get_model() {return model_;}

  void put_shadow(SDL_Point point);

  void play_placed_tiles(bool is_player);

  void render() const;

  [[nodiscard]] std::span<const Tile> already_played_view() const { return already_played_; }

private:
  void clear_rect(const SDL_Rect& rect);

  void place_highlight(const SDL_Rect& rect, bool is_player);

  SDL_Renderer *renderer_;
  BlankTileReplacer *replacer_;
  Texture board_texture_;
  std::vector<Tile> recently_placed_, already_played_, blanks_;
  std::vector<Tile> computer_highlighted_tiles_;
  Tile taken_;
  SDL_Point last_taken_point_{};
  std::optional<SDL_Rect> shadow_;
  BoardModel model_;
};

#endif // BOARD_H
