#include "Playing.h"
#include "BlankTileReplacer.h"
#include "ConfirmationDialog.h"
#include "Game.h"
#include "battery/embed.hpp"
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

constexpr SDL_Rect kRestartButtonRect{.x = 1023, .y = 745, .w = 120, .h = 64};

constexpr SDL_Rect kSwapButtonRect{.x = 860, .y = 745, .w = 120, .h = 64};

constexpr SDL_Rect kSkipButtonRect{.x = 860, .y = 820, .w = 120, .h = 64};

constexpr SDL_Rect kHintButtonRect{.x = 1023, .y = 820, .w = 120, .h = 64};

constexpr SDL_Rect kCloseButtonRect{
    .x = constants::kWindowWidth - 30, .y = 10, .w = 25, .h = 25};

constexpr int kMaxSkipsInARow = 6; // the 'six-zero  rule'

namespace {
[[nodiscard]] constexpr std::string_view invalid_placement_error_to_string(Solver::InvalidPlacementError error) {
  using enum Solver::InvalidPlacementError;
  switch(error) {
    case NO_TILES_PROVIDED: return "At least one tile must be played";
    case MIDDLE_SQUARE_NOT_FILLED: return "The middle square is not filled";
    case NO_ADJACENT_TILE: return "At least one tile must be next to an already existing tile";
    case NOT_STRAIGHT_LINE: return "Tiles must be in a straight line";
    case NOT_CONTIGUOUS: return "Tiles must be contiguous (no spaces between them)";
    default: std::unreachable();
  }
}
} // namespace

Playing::Playing(const AssetPool& assets, ButtonMaker& button_maker) :
  background_{assets.get(TextureType::BOARD)},
  tile_bag_{Game::renderer()},
  blank_replacer_(board_, *this),
  board_{Game::renderer(), blank_replacer_},
  solver_{board_.get_model(), dictionary_},
  rack_{Game::renderer()},
  confirm_dialog_(*this, assets, button_maker),
  tile_swapper_(*this, assets, tile_bag_),
  shuffle_{assets.get(TextureType::SHUFFLE_BUTTON), kShuffleButtonRect, std::bind_front(&Rack::shuffle, &rack_)},
  recall_{assets.get(TextureType::RECALL_BUTTON), kRecallButtonRect, std::bind_front(&Playing::recall_tiles, this)},
  buttons_{
    shuffle_,
    {assets.get(TextureType::ENTER_BUTTON), kEnterButtonRect, std::bind_front(&Playing::play_turn, this)}, // Enter
    {button_maker.make_text_button(Game::renderer(), "Restart", kRestartButtonRect, std::bind_front(&Playing::ask_to_restart, this))}, // Restart
    {button_maker.make_text_button(Game::renderer(), "Swap", kSwapButtonRect, [this]{tile_swapper_.ask(get_snapshot(false), rack_.get_tiles());})}, //Swap
    {button_maker.make_text_button(Game::renderer(), "Skip", kSkipButtonRect, std::bind_front(&Playing::skip_turn, this))}, // Skip
    {button_maker.make_text_button(Game::renderer(), "Hint", kHintButtonRect, std::bind_front(&Playing::put_best_move, this))}, // Hint 
    {assets.get(TextureType::X_BUTTON), kCloseButtonRect, [this]{ confirm_dialog_.ask("Exit to Main Menu?", &Game::pop_until<MainMenu>);}} // Close
  }, 
  scoreboard_(Game::renderer(), assets),
  player_word_outliner_{Game::renderer(), assets},
  computer_word_outliner_{Game::renderer(), assets},
  game_over_{*this, assets, button_maker},
  counter_{Game::renderer(), assets, tile_bag_.tiles_view()}
{
  utility::log("Loading dictionary...");
  std::ignore = glz::read_binary_untagged(dictionary_, b::embed<"assets/dict.bin">().view());
  utility::log("Dictionary loaded");
  buttons_[ENTER].disable();
  tile_bag_.shuffle();
  fill_player_rack();
  fill_computer_rack();
  utility::log("Playing state initialized");
  players_turn_ ? start_player_turn() : play_opponent_turn();
}

void Playing::show_shuffle_button() {
  buttons_[SHUFFLE_OR_RECALL] = shuffle_;
}

void Playing::show_recall_button() {
  buttons_[SHUFFLE_OR_RECALL] = recall_;
}

Tile *Playing::take_tile()  {
  Tile *tile = rack_.take_from(Mouse::pos());
  if (tile != nullptr) {
    picked_up_from_ = RACK;
  } else if (tile = board_.take_from_board(Mouse::pos()); tile != nullptr) {
    picked_up_from_ = BOARD;
  }
  return tile;
}

void Playing::click_button() const  {
  std::visit([this](auto&& hovered) {
    if constexpr(std::same_as<std::remove_cvref_t<decltype(hovered)>, Button*>) {
      if(hovered != &buttons_[SHUFFLE_OR_RECALL]) {
        Mouse::click_hovered(hovered);
      } else {
        hovered->click();
      }
    }
  }, hovered_);
}

void Playing::render_objects() const {
  SDL_RenderCopy(Game::renderer(), background_, nullptr, nullptr);
  board_.render();
  computer_word_outliner_.render();
  player_word_outliner_.render();
  rack_.render(Game::renderer());
  std::ranges::for_each(buttons_, [](const auto& btn) static {btn.render(Game::renderer());});
  scoreboard_.render();
  counter_.render(Game::renderer());
  if (selected_tile_ != nullptr) {
    selected_tile_->render(Game::renderer());
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
  computer_tiles_.clear();
  counter_.reset(Game::renderer());
  tile_bag_.reset();
  assert(tile_bag_.tiles_left() == constants::kBagTileAmount);
  board_.reset();
  rack_.reset();
  saved_best_move_.reset();
  scoreboard_.reset();
  fill_player_rack();
  fill_computer_rack();
  show_shuffle_button();
  player_used_hints_ =  false;
  player_word_outliner_.set_hidden(true);
  computer_word_outliner_.set_hidden(true);
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
  buttons_[ENTER].disable();
  player_has_valid_placement_ = false;
  computer_word_outliner_.set_hidden(scoreboard_.get_score(Scoreboard::Player::COMPUTER) == 0);
}

void Playing::ask_to_restart() {
  if(board_.empty()){
      restart_game();
  } else{
    confirm_dialog_.ask("Restart the game?", std::bind_front(&Playing::restart_game, this));
  }
}

void Playing::swap_tiles(std::bitset<constants::kRackTileAmount> positions) {
  assert(players_turn_);
  std::vector<Tile> unwanted_tiles;
  unwanted_tiles.reserve(positions.count());
  for (auto i = 0U; i < constants::kRackTileAmount; ++i) {
    if (positions.test(i)) {
      unwanted_tiles.push_back(rack_.take_tile(i));
    }
  }
  auto new_tiles = tile_bag_.swap(unwanted_tiles);
  std::ranges::for_each(new_tiles, [this](const Tile& tile){rack_.put(tile);});
  switch_turns();
}

void Playing::play_turn() {
  assert(players_turn_);
  int word_value = player_word_outliner_.get_last_displayed();
  utility::log("Player plays word(s) worth {} points", word_value);
  skipped_turns_in_a_row_ = 0;
  scoreboard_.add_score(Scoreboard::Player::HUMAN, word_value);
  counter_.update_count(Game::renderer(), board_.recently_placed_view());
  board_.play_placed_tiles(true);
  fill_player_rack();
  show_shuffle_button();
  switch_turns();
}

Texture Playing::get_snapshot(bool capture_rack) {
  const auto render_buttons = [](auto& buttons) static {
    std::ranges::for_each(buttons, [](const auto& btn) static { btn.render(Game::renderer());});
  };
  Texture result{SDL_CreateTexture(Game::renderer(), SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET,
      constants::kWindowWidth, constants::kWindowHeight)};
  SDL_SetRenderTarget(Game::renderer(), result.get());
  SDL_RenderCopy(Game::renderer(), background_, nullptr, nullptr);
  if(capture_rack){
    rack_.render(Game::renderer());
    render_buttons(buttons_);
  } else {
    auto no_rack_buttons = buttons_ |
      std::views::enumerate |
      std::views::filter([](auto&& pair) static {
        return std::get<0>(pair) != SHUFFLE_OR_RECALL && std::get<0>(pair) != ENTER;
      }) |
      std::views::values;
    render_buttons(no_rack_buttons);
  }
  player_word_outliner_.render();
  computer_word_outliner_.render();
  board_.render();
  scoreboard_.render();
  counter_.render(Game::renderer());
  SDL_SetRenderTarget(Game::renderer(), nullptr);
  return result;
  }

void Playing::switch_turns() {
  // conditions for the game to end
  if ((rack_.empty() or computer_tiles_.empty()) or (skipped_turns_in_a_row_ == kMaxSkipsInARow)) {
    if constexpr(constants::debug) {
      if(skipped_turns_in_a_row_ == kMaxSkipsInARow) {
        utility::log("Max skips in a row limit ({}/{}) reached", skipped_turns_in_a_row_, kMaxSkipsInARow);
      } else {
        std::string_view player_to_empty = rack_.empty() ? "Player" : "Opponent";
        utility::log("{} emptied all their tiles", player_to_empty);
      }
    }
    int player_score = scoreboard_.get_score(Scoreboard::Player::HUMAN);
    int computer_score = scoreboard_.get_score(Scoreboard::Player::COMPUTER);
    int player_rack_val = std::ranges::fold_left(rack_.tile_view() | std::views::transform(&Tile::value), 0, std::plus{});
    int opp_rack_val = std::ranges::fold_left(computer_tiles_ | std::views::transform(&Tile::value), 0, std::plus{});
    game_over_.show(player_score, computer_score, player_rack_val, opp_rack_val, player_used_hints_);
    return;
  }
  players_turn_^=1U;
  saved_best_move_.reset();
  players_turn_ ? start_player_turn() : play_opponent_turn();
}

void Playing::start_player_turn() {
  utility::log("Player turn start");
  std::ranges::for_each(buttons_, [](auto& btn){btn.enable();});
  buttons_[ENTER].disable();
}

void Playing::play_opponent_turn() {
  utility::log("Opponent turn start");
  assert(not board_.has_recently_placed_tiles());
  player_has_valid_placement_ = false;
  player_word_outliner_.set_hidden(true);
  for(auto [index, button] : std::views::enumerate(buttons_)) {
    if (index == SHUFFLE_OR_RECALL || index == RESTART) {
      continue;
    }
    button.disable();
  }
  assert(not saved_best_move_);
  saved_best_move_.emplace(solver_.get_best_move(computer_tiles_));
  if(saved_best_move_->info.score==0){
    computer_word_outliner_.set_hidden(true);
    ++skipped_turns_in_a_row_;
    utility::log("Opponent skips turn, {}/{} allowed skips in a row", skipped_turns_in_a_row_, kMaxSkipsInARow);
    switch_turns();
    return;
  }
  utility::log("Opponent plays word(s) worth {} points", saved_best_move_->info.score);
  computer_word_outliner_.set_hidden(false);
  for(auto [pos, letter, is_blank] : saved_best_move_->tiles) {
    auto it = std::ranges::find(computer_tiles_, is_blank ? constants::kTileBlankChar : letter, &Tile::letter);
    assert(it!=computer_tiles_.end());
    board_.put_on_board(pos, *it, false);
    if (is_blank) {
      blank_replacer_.replace_blank(pos, letter);
    }
    std::iter_swap(it, computer_tiles_.rbegin()); computer_tiles_.pop_back();
  }
  auto [word_begin_pos, word_end_pos, total_score] = saved_best_move_->info;
  scoreboard_.add_score(Scoreboard::Player::COMPUTER, total_score);
  SDL_Point begin = to_point(word_begin_pos);
  SDL_Point end = to_point(word_end_pos);
  computer_word_outliner_.outline(begin, end, total_score);
  counter_.update_count(Game::renderer(), board_.recently_placed_view());
  board_.play_placed_tiles(false);
  fill_computer_rack();
  skipped_turns_in_a_row_ = 0;
  switch_turns();
}

void Playing::skip_turn() {
  confirm_dialog_.ask("Skip your turn?", [this] {
    ++skipped_turns_in_a_row_;
    utility::log("Skipping turn, {}/{} allowed skips in a row", skipped_turns_in_a_row_, kMaxSkipsInARow);
    recall_tiles();
    switch_turns();
  });
}

void Playing::handle_valid_placement(bool correct_words, Row_Col begin, Row_Col end, std::int32_t score) {
  player_has_valid_placement_ = true;
  player_word_outliner_.outline(to_point(begin), to_point(end), score, correct_words);
  player_word_outliner_.set_hidden(score == 0);
  computer_word_outliner_.set_hidden(player_word_outliner_.begin_pos() == computer_word_outliner_.begin_pos() || 
      scoreboard_.get_score(Scoreboard::Player::COMPUTER) == 0);
}

void Playing::evaluate_board() {
  Solver::Evaluation results = solver_.get_board_evaluation(board_.recently_placed_view());
  if (results.has_value()) {
    utility::log("Valid word(s) worth {} points", results->score);
    handle_valid_placement(true, results->begin, results->end, results->score);
    if (players_turn_){
      buttons_[ENTER].enable();
    }
  } else {
    std::visit([this](const auto& error){
      using T = std::remove_cvref_t<decltype(error)>;
      if constexpr(std::same_as<T, Solver::InvalidPlacementError>) {
        player_has_valid_placement_ = false;
        player_word_outliner_.set_hidden(true);
        computer_word_outliner_.set_hidden(scoreboard_.get_score(Scoreboard::Player::COMPUTER) == 0);
        utility::log("{}", invalid_placement_error_to_string(error));
      }
      else if constexpr(std::same_as<T, Solver::ValidPlacementInvalidWords>) {
        handle_valid_placement(false, error.placement.begin, error.placement.end, error.placement.score);
        utility::log("INVALID WORDS: {}", error.invalid_words);
      } else {
        static_assert(false, "Variant alternative not handled!");
      }
    }, results.error());
    buttons_[ENTER].disable();
  }
}

void Playing::put_best_move() {
  // only calculate the best move once
  if(not saved_best_move_){
    utility::log("Computing best move for player...");
    std::vector<Tile> no_gap_tiles{rack_.get_tiles()};
    std::erase_if(no_gap_tiles, [](const Tile &tile) { return tile.letter == constants::kTileGapChar; });
    saved_best_move_.emplace(solver_.get_best_move(no_gap_tiles));
    utility::log("Best move computed: worth {} points", saved_best_move_->info.score);
  } else {
    if constexpr(constants::debug) {
      utility::log("Using already computed best move worth {} points", saved_best_move_->info.score);
    }
  }
  auto [word_begin_pos, word_end_pos, total_score] = saved_best_move_->info;
  if (total_score == 0) {
    utility::log("Player cannot make any valid words!");
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
    buttons_[ENTER].enable();
    player_used_hints_ = player_has_valid_placement_ = true;
    player_word_outliner_.set_hidden(false);
    if(computer_word_outliner_.begin_pos() == player_word_outliner_.begin_pos()){
      computer_word_outliner_.set_hidden(true);
    }
  }
}

void Playing::handle_event(const SDL_Event& event) {
  switch (event.type) {
    case SDL_MOUSEMOTION:
      if (selected_tile_ != nullptr) {
        selected_tile_->move(Mouse::pos() - tile_offset_);
        board_.put_shadow(Mouse::pos());
        (picked_up_from_ == RACK) ? rack_.swap_tiles(Mouse::pos()) : rack_.make_room_for_tile(Mouse::pos());
      } else {
        hovered_ = Mouse::handle_hovering(buttons_, rack_.tile_view(), board_.recently_placed_view());
      }
      return;
    case SDL_MOUSEBUTTONDOWN:
      if (event.button.button != SDL_BUTTON_LEFT) {
        return;
      }
      if (selected_tile_ = take_tile(); selected_tile_ != nullptr) {
        Mouse::set_hand_cursor();
        player_word_outliner_.set_hidden(true);
        tile_offset_ = Mouse::pos() - selected_tile_->point();
        selected_tile_->unhover();
        std::visit([](auto* hovered){hovered->unhover();}, hovered_); // can't be null because hovered_ == selected_tile
      }
      return;
    case SDL_MOUSEBUTTONUP:
      if (event.button.button != SDL_BUTTON_LEFT) {
        return;
      }
      if (selected_tile_ == nullptr) {
        click_button();
      } else if (rack_.put(Mouse::pos(), *selected_tile_) or board_.put_on_board(*selected_tile_)) {
        evaluate_board();
      } else {
        Mouse::set_default_cursor();
        picked_up_from_ == RACK ? rack_.return_tile() : board_.return_tile();
      }
      if (board_.has_recently_placed_tiles()) {
        show_recall_button();
        buttons_[SWAP].disable();
        buttons_[HINT].disable();
      } else {
        show_shuffle_button();
        if (players_turn_){
          buttons_[SWAP].enable();
          buttons_[HINT].enable();
        }
      }
      selected_tile_ = nullptr;
      player_word_outliner_.set_hidden(!player_has_valid_placement_);
      return;
    case SDL_KEYDOWN:
      if (event.key.keysym.sym == SDLK_RETURN and buttons_[ENTER].is_enabled()) {
        play_turn();
      }
      return;
    default:
      return;
  }
}
