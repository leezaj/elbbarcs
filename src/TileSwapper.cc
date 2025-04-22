#include "Game.h"
#include "Mouse.h"
#include "TileSwapper.h"
#include "Playing.h"
#include "types.h"
#include <SDL2/SDL_render.h>
#include <algorithm>

namespace {
} // namespace

static constexpr SDL_Rect kCloseButtonRect{
  .x = constants::kRackTilePositions.front().x - 100,
  .y = constants::kBoardDims + 18,
  .w = 50,
  .h = 50
};

static constexpr SDL_Rect kConfirmButtonRect{
  .x = constants::kRackTilePositions.back().x + constants::kRackButtonGap,
  .y = constants::kBoardDims + 25,
  .w = 70,
  .h = 41};

static constexpr SDL_Point kTilesLeftPos{675, 760};
static constexpr SDL_Point kSelectedTilesPos{675, 785};

static constexpr int kFontSize = 18;

TileSwapper::TileSwapper(Playing &playing_state, const AssetPool &assets, const TileBag& bag) : 
  buttons_{
  Button{assets.get(TextureType::X_BUTTON), kCloseButtonRect, [this] ->  void {
      selected_tiles_.reset();
      Game::pop_until<Playing>();
      buttons_[CONFIRM].disable();
    }},
  Button{assets.get(TextureType::ENTER_BUTTON), kConfirmButtonRect, [&playing_state, this] -> void {
      playing_state.swap_tiles(selected_tiles_);
      buttons_[CLOSE].click();
     }}
  },
  bag_{&bag},
  remaining_text_{Game::renderer(), assets.get(FontType::MOULDY_CHEESE), kFontSize, constants::kColorBeige},
  selected_text_{Game::renderer(), assets.get(FontType::MOULDY_CHEESE), kFontSize, constants::kColorBeige}
{
  update_tiles_left_text();
  utility::log("Tile swapper initialized");
}

void TileSwapper::ask(Texture background, std::vector<Tile> tiles) {
  background_ = std::move(background);
  SDL_SetTextureColorMod(background_.get(), constants::kDimRGB, constants::kDimRGB, constants::kDimRGB);
  tiles_ = std::move(tiles);
  update_tiles_left_text();
  update_selected_tiles_text(0);
  Game::push_game_state(this);
  buttons_[CONFIRM].disable();
  hovered_object_ = Mouse::handle_hovering(buttons_, tiles_);
}

void TileSwapper::update_tiles_left_text() {
  if (auto bag_count = static_cast<int>(bag_->tiles_left()); bag_count != tiles_left_) {
    tiles_left_ = bag_count;
    remaining_text_.update(Game::renderer(), constants::kColorBeige, std::to_string(bag_count) + " tiles left", kTilesLeftPos);
  }
}

void TileSwapper::update_selected_tiles_text(int count) {
  selected_str_[0] = static_cast<char>(count + '0');
  selected_text_.update(Game::renderer(), constants::kColorBeige, selected_str_, kSelectedTilesPos);
}

void TileSwapper::render_objects() const {
  SDL_RenderCopy(Game::renderer(), background_.get(), nullptr, nullptr);
  std::ranges::for_each(tiles_, [](const Tile &tile) static { tile.render(Game::renderer()); });
  std::ranges::for_each( buttons_, [](const Button &button) static { button.render(Game::renderer()); });
  remaining_text_.render(Game::renderer());
  selected_text_.render(Game::renderer());
}

void TileSwapper::handle_event(const SDL_Event& event) {
  static constexpr std::uint8_t kToggleHeight = 20;
  switch(event.type) {
    case SDL_MOUSEMOTION:
      hovered_object_ = Mouse::handle_hovering(buttons_, tiles_);
      return;
    case SDL_KEYDOWN:
      if (event.key.keysym.sym == SDLK_ESCAPE) {
        buttons_[CLOSE].click();
      } else if (event.key.keysym.sym == SDLK_RETURN and selected_tiles_.any()){
        buttons_[CONFIRM].click();
      }
      return;
    case SDL_MOUSEBUTTONUP:
      std::visit(utility::Overload {
        [this](Tile* hovered_tile) { // tile case
          if(hovered_tile == nullptr) {
            return;
          }
          const auto idx = static_cast<size_t>(std::distance(tiles_.data(), hovered_tile));
          hovered_tile->rect.y += kToggleHeight * (1 - 2 * static_cast<int>(selected_tiles_.flip(idx).test(idx)));
          const int selected = static_cast<int>(selected_tiles_.count());
          update_selected_tiles_text(selected);
          selected > 0 and tiles_left_ >= selected ? buttons_[CONFIRM].enable() : buttons_[CONFIRM].disable();
        },
        [](Button* hovered_button) static { // button case
          if(hovered_button != nullptr) {
          Mouse::click_hovered(hovered_button);
          }
        }
      }, hovered_object_);
      [[fallthrough]];
    default:
      return;
  }
} 
