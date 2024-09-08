#include "Playing.h"
#include "BlankTileReplacer.h"
#include "ConfirmationDialog.h"
#include "constants.h"
#include "utility.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <algorithm>
#include <cassert>
#include <glaze/glaze.hpp>

constexpr SDL_Rect kShuffleButtonRect{
  .x = constants::kRackTilePositions.front().x - constants::kRackButtonGap, 
  .y = constants::kBoardDims + 15, 
  .w = 69, 
  .h = 57
};

constexpr SDL_Rect kEnterButtonRect{
  .x = constants::kRackTilePositions.back().x +
  constants::kRackButtonGap,
  .y = constants::kBoardDims + 25,
  .w = 70,
  .h = 41
};

constexpr SDL_Rect kRecallButtonRect{
  .x = constants::kRackTilePositions.front().x -
  constants::kRackButtonGap,
  .y = constants::kBoardDims + 15,
  .w = 69,
  .h = 47
};

static const auto kSerializedDictPath = (AssetPool::assets_path()/"dict.bin");

constexpr SDL_Rect kRestartButtonRect{.x = 1023, .y = 745, .w = 120, .h = 64};

constexpr SDL_Rect kSwapButtonRect{.x = 860, .y = 745, .w = 120, .h = 64};

constexpr SDL_Rect kSkipButtonRect{.x = 860, .y = 820, .w = 120, .h = 64};

constexpr SDL_Rect kHintButtonRect{.x = 1023, .y = 820, .w = 120, .h = 64};

constexpr SDL_Rect kCloseButtonRect{
    .x = constants::kWindowWidth - 30, .y = 10, .w = 25, .h = 25};

constexpr int kMaxSkipsInARow = 6; // the 'six-zero  rule'

Playing::Playing(SDL_Renderer *rend, Mouse& mouse, const AssetPool& assets, GameStateManager& manager, ButtonMaker& button_maker) :
  GameState{rend, mouse},
  background_{assets.get(TextureType::BOARD)},
  tile_bag_{rend},
  blank_replacer_(rend, mouse, manager, board_, *this),
  board_{rend, blank_replacer_},
  solver_{board_.get_model(), dictionary_},
  rack_{rend},
  confirm_dialog_(rend, mouse, manager, *this, assets, button_maker),
  tile_swapper_(rend, mouse, manager, *this, assets, tile_bag_),
  shuffle_{assets.get(TextureType::SHUFFLE_BUTTON), kShuffleButtonRect, std::bind_front(&Rack::shuffle, &rack_)},
  enter_{assets.get(TextureType::ENTER_BUTTON), kEnterButtonRect, std::bind_front(&Playing::play_turn, this)},
  recall_{assets.get(TextureType::RECALL_BUTTON), kRecallButtonRect, std::bind_front(&Playing::recall_tiles, this)},
  restart_{button_maker.make_text_button(rend, "Restart", kRestartButtonRect, std::bind_front(&Playing::ask_to_restart, this))},
  swap_{button_maker.make_text_button(rend, "Swap", kSwapButtonRect, [this]{tile_swapper_.ask(get_snapshot(false), rack_.get_tiles());})},
  skip_{button_maker.make_text_button(rend, "Skip", kSkipButtonRect, std::bind_front(&Playing::skip_turn, this))},
  hint_{button_maker.make_text_button(rend, "Hint", kHintButtonRect, std::bind_front(&Playing::put_best_move, this))},
  close_{assets.get(TextureType::X_BUTTON), kCloseButtonRect, [this, &manager]{
      confirm_dialog_.ask("Exit to Main Menu?", [&manager] {
        while(manager.size()!=1) {
          manager.pop();
        }
      });
  }},
  buttons_{&shuffle_, &enter_, &restart_, &swap_, &skip_, &hint_, &close_},
  scoreboard_(rend, assets),
  player_word_outliner_{rend, assets},
  computer_word_outliner_{rend, assets},
  game_over_{rend, mouse, manager, *this, assets, button_maker},
  counter_{rend, assets, tile_bag_.tiles_view()}
{
  std::ignore = glz::read_file_binary(dictionary_, kSerializedDictPath.c_str(), std::string{});
  enter_.disable();
  tile_bag_.shuffle();
  fill_player_rack();
  fill_computer_rack();
  players_turn_ ? start_player_turn() : play_opponent_turn();
}

void Playing::show_shuffle_button() {
  buttons_[0] = &shuffle_;
}

void Playing::show_recall_button() {
  buttons_[0] = &recall_;
}

Tile *Playing::take_tile()  {
  Tile *tile = rack_.take_from(mouse_pos());
  if (tile != nullptr) {
    picked_up_from_ = RACK;
  } else if (tile = board_.take_from_board(mouse_pos()); tile != nullptr) {
    picked_up_from_ = BOARD;
  }
  return tile;
}

const Button* Playing::button_at_pos() const  {
  if(const auto button = std::ranges::find_if(buttons_, [this](const auto& btn){ 
      return contains(btn->rectangle(), mouse_pos()) and btn->is_enabled(); }); button!=buttons_.end()) {
    return *button;
  }
  return nullptr;
}

void Playing::click_button() const  {
  if(const Button *button = button_at_pos(); button != nullptr) {
    button->click();
  }
}

void Playing::handle_hovering()  {
  if (hovered_ != nullptr) {
    hovered_->unhover();
  }
  const bool hovering =
      (hovered_ = board_.tile_at_pos(mouse_pos())) != nullptr or
      ((hovered_ = rack_.tile_at_pos(mouse_pos()))) != nullptr or
      ((hovered_ = button_at_pos())) != nullptr;
  if (hovering) {
    if (not mouse_down_) {
      hovered_->hover();
    }
    set_hand_cursor();
  } else {
    set_default_cursor();
  }
}

void Playing::render_objects() const {
  background_.render(renderer());
  board_.render();
  if (display_player_points_) {
    player_word_outliner_.render();
  } else if (display_computer_points_) {
    computer_word_outliner_.render();
  }
  rack_.render(renderer());
  std::ranges::for_each(buttons_, [this](auto* btn){btn->render(renderer());});
  scoreboard_.render();
  counter_.render(renderer());
  if (selected_tile_ != nullptr) {
    selected_tile_->render(renderer());
  }
}

void Playing::fill_player_rack()  {
  const auto tiles_to_put = rack_.missing_tiles();
  for(auto i = 0; i<tiles_to_put and not tile_bag_.empty(); ++i) {
    rack_.put(tile_bag_.take_from());
  }
}

void Playing::fill_computer_rack() {
  auto missing_tiles = constants::kRackTileAmount - computer_tiles_.size();
  for(auto i = 0UZ; i<missing_tiles and not tile_bag_.empty(); ++i) {
    computer_tiles_.push_back(tile_bag_.take_from());
  }
}

void Playing::restart_game() {
  std::vector<Tile> tiles_to_put_back;
  tiles_to_put_back.reserve(rack_.num_of_tiles() + board_.num_of_tiles() + computer_tiles_.size());
  while (not board_.empty()) {
    tiles_to_put_back.push_back(board_.take_tile());
  }
  while (not rack_.empty()) {
    tiles_to_put_back.push_back(rack_.take_tile());
  }
  tiles_to_put_back.insert(tiles_to_put_back.end(), computer_tiles_.begin(), computer_tiles_.end());
  computer_tiles_.clear();
  counter_.reset(renderer());
  tile_bag_.put_tiles(std::move(tiles_to_put_back));
  tile_bag_.shuffle();
  assert(tile_bag_.tiles_left() == constants::kBagTileAmount);
  board_.reset();
  saved_best_move_.reset();
  scoreboard_.reset();
  fill_player_rack();
  fill_computer_rack();
  show_shuffle_button();
  player_used_hints_ = display_player_points_ = display_computer_points_ = false;
  skipped_turns_in_a_row_ = 0;
  players_turn_ = Random::coin_flip();
  players_turn_ ? start_player_turn() : play_opponent_turn();
}

void Playing::recall_tiles() {
  const auto num_to_recall = rack_.missing_tiles();
  for(auto i=0; i<num_to_recall and board_.has_recently_placed_tiles(); ++i) {
    rack_.put(board_.take_oldest_placed());
  }
  show_shuffle_button();
  enter_.disable();
  player_has_valid_placement_ = display_player_points_ = false;
}

void Playing::ask_to_restart() {
  if(board_.empty()){
      restart_game();
  } else{
    confirm_dialog_.ask("Restart the game?", std::bind_front(&Playing::restart_game, this));
  }
}

void Playing::swap_tiles(const std::bitset<constants::kRackTileAmount> &positions) {
  assert(players_turn_);
  std::vector<Tile> unwanted_tiles;
  unwanted_tiles.reserve(positions.count());
  for (auto i = 0U; i < constants::kRackTileAmount; ++i) {
    if (positions.test(i)) {
      unwanted_tiles.push_back(rack_.take_tile(i));
    }
  }
  fill_player_rack();
  tile_bag_.put_tiles(std::move(unwanted_tiles));
  tile_bag_.shuffle();
  switch_turns();
}

void Playing::play_turn() {
  assert(players_turn_);
  int word_value = player_word_outliner_.get_last_displayed();
  skipped_turns_in_a_row_ = 0;
  scoreboard_.add_score(Scoreboard::Player::HUMAN, word_value);
  board_.play_placed_tiles(true);
  fill_player_rack();
  switch_turns();
}

Texture Playing::get_snapshot(bool capture_rack) {
  const auto render_buttons = [this](auto&& buttons) {
    std::ranges::for_each(buttons, [this](const auto* btn) { btn->render(renderer());});
  };
  Texture result{SDL_CreateTexture(renderer(), SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET,
      constants::kWindowWidth, constants::kWindowHeight)};
  SDL_SetRenderTarget(renderer(), result.get());
  background_.render(renderer());
  if(capture_rack){
    rack_.render(renderer());
    render_buttons(buttons_);
  } else {
    render_buttons(buttons_ | std::views::filter([this](auto *btn) {
      return btn != &shuffle_ and btn != &enter_ and btn != &recall_;
    }));
  }
  board_.render();
  scoreboard_.render();
  counter_.render(renderer());
  SDL_SetRenderTarget(renderer(), nullptr);
  return result;
  }

void Playing::switch_turns() {
  // conditions for the game to end
  if ((rack_.empty() or computer_tiles_.empty()) or (skipped_turns_in_a_row_ == kMaxSkipsInARow)) {
    int player_score = scoreboard_.get_score(Scoreboard::Player::HUMAN);
    int computer_score = scoreboard_.get_score(Scoreboard::Player::COMPUTER);
    auto player_vals = rack_.tile_view() | std::views::transform(&Tile::value);
    auto computer_vals = computer_tiles_ | std::views::transform(&Tile::value);
    auto player_sum = std::reduce(player_vals.begin(), player_vals.end());
    auto computer_sum = std::reduce(computer_vals.begin(), computer_vals.end());
    game_over_.show(player_score, computer_score, player_sum, computer_sum, player_used_hints_);
    return;
  }
  counter_.update_count(renderer(), board_.already_played_view());
  players_turn_^=1U;
  saved_best_move_.reset();
  players_turn_ ? start_player_turn() : play_opponent_turn();
}

void Playing::start_player_turn() {
  recall_tiles();
  std::ranges::for_each(buttons_, [](auto btn){btn->enable();});
  enter_.disable();
}

void Playing::play_opponent_turn() {
  assert(not board_.has_recently_placed_tiles());
  display_player_points_ = player_has_valid_placement_ = false;
  for(auto& btn: buttons_) {
    if (btn == &shuffle_ || btn == &restart_ || btn == &recall_) {
      continue;
    }
    btn->disable();
  }
  assert(not saved_best_move_);
  saved_best_move_.emplace(solver_.get_best_move(computer_tiles_));
  if(saved_best_move_->info.total_score==0){
    ++skipped_turns_in_a_row_;
    switch_turns();
    return;
  }
  for(auto [pos, letter, is_blank] : saved_best_move_->tiles) {
    auto it = std::ranges::find(computer_tiles_, is_blank ? constants::kTileBlankChar : letter, &Tile::letter);
    assert(it!=computer_tiles_.end());
    board_.put_on_board(pos, *it, false);
    if (is_blank) {
      blank_replacer_.replace_blank(pos, letter);
    }
    computer_tiles_.erase(it);
  }
  auto [_, total_score, word_begin_pos, word_end_pos] = saved_best_move_->info;
  scoreboard_.add_score(Scoreboard::Player::COMPUTER, total_score);
  SDL_Point begin = to_point(word_begin_pos);
  SDL_Point end = to_point(word_end_pos);
  computer_word_outliner_.outline(begin, end,total_score);
  display_computer_points_ = true;
  board_.play_placed_tiles(false);
  fill_computer_rack();
  skipped_turns_in_a_row_ = 0;
  switch_turns();
}

void Playing::skip_turn() {
  confirm_dialog_.ask("Skip your turn?", [this] {
    ++skipped_turns_in_a_row_;
    recall_tiles();
    switch_turns();
  });
}

void Playing::evaluate_board() {
  auto results = solver_.get_board_evaluation(board_.placed_tile_positions());
  player_has_valid_placement_ = display_player_points_ = results.has_value();
  if(player_has_valid_placement_) {
    auto [has_valid_words, points, begin, end] = *results;
    has_valid_words and players_turn_ ? enter_.enable() : enter_.disable();
    player_word_outliner_.outline(to_point(begin), to_point(end), points, has_valid_words);
  } else {
    enter_.disable();
  }
}

void Playing::put_best_move() {
  // only calculate the best move once
  if(not saved_best_move_){
    std::vector<Tile> no_gap_tiles{rack_.get_tiles()};
    std::erase_if(no_gap_tiles, [](const Tile &tile) {
      return tile.letter() == constants::kTileGapChar;
    });
    saved_best_move_.emplace(solver_.get_best_move(no_gap_tiles));
  }
  auto [_, total_score, word_begin_pos, word_end_pos] = saved_best_move_->info;
  if (total_score == 0) {
    return;
  }
  for(auto [pos, letter, is_blank] : saved_best_move_->tiles) {
    if(is_blank){
      board_.put_on_board(pos,rack_.take_tile(constants::kTileBlankChar),true);
      blank_replacer_.replace_blank(pos, letter);
    } else {
      board_.put_on_board(pos, rack_.take_tile(letter), true);
    }
  }
  if (board_.has_recently_placed_tiles()) {
    SDL_Point begin = to_point(word_begin_pos);
    SDL_Point end = to_point(word_end_pos);
    player_word_outliner_.outline(begin, end, total_score, true);
    enter_.enable();
    player_used_hints_ = player_has_valid_placement_ = display_player_points_ = true;
    display_computer_points_ = false;
  }
}

void Playing::handle_event(const SDL_Event& event) {
  switch (event.type) {
    case SDL_MOUSEMOTION:
      set_mouse_pos({.x = event.motion.x, .y = event.motion.y});
      if (selected_tile_ != nullptr) {
        selected_tile_->move(mouse_pos() - tile_offset_);
        board_.put_shadow(mouse_pos());
        (picked_up_from_ == RACK) ? rack_.swap_tiles(mouse_pos()) : rack_.make_room_for_tile(mouse_pos());
      } else {
        handle_hovering();
      }
      return;
    case SDL_MOUSEBUTTONDOWN:
      if (event.button.button != SDL_BUTTON_LEFT) {
        return;
      }
      if (selected_tile_ = take_tile(); selected_tile_ != nullptr) {
        display_player_points_ = false;
        tile_offset_ = mouse_pos() - selected_tile_->point();
        selected_tile_->unhover();
      } else if (hovered_ != nullptr){
        hovered_->unhover();
      }
      mouse_down_ = true;
      return;
    case SDL_MOUSEBUTTONUP:
      if (event.button.button != SDL_BUTTON_LEFT) {
        return;
      }
      if (selected_tile_ == nullptr) {
        click_button();
      } else if (rack_.put(mouse_pos(), *selected_tile_) or board_.put_on_board(*selected_tile_)) {
        evaluate_board();
      } else {
        set_default_cursor();
        picked_up_from_ == RACK ? rack_.return_tile() : board_.return_tile();
      }
      if (board_.has_recently_placed_tiles()) {
        show_recall_button();
        swap_.disable();
        hint_.disable();
      } else {
        show_shuffle_button();
        if (players_turn_){
          swap_.enable();
          hint_.enable();
        }
      }
      selected_tile_ = nullptr;
      mouse_down_ = false;
      display_computer_points_ = skipped_turns_in_a_row_ == 0 and not board_.empty() and not board_.has_recently_placed_tiles();
      display_player_points_ = player_has_valid_placement_;
      return;
    case SDL_KEYDOWN:
      if (event.key.keysym.sym == SDLK_RETURN and enter_.is_enabled()) {
        play_turn();
      }
      return;
    default:
      return;
  }
}
