#include "TileSwapper.h"
#include "Playing.h"
#include "types.h"
#include <SDL2/SDL_render.h>
#include <algorithm>

namespace {
enum Buttons : std::uint8_t {
  CLOSE = 0,
  CONFIRM = 1
};
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

TileSwapper::TileSwapper(SDL_Renderer* rend, Mouse &mouse, GameStateManager &manager, Playing &playing_state, const AssetPool &assets, const TileBag& bag) : 
  GameState(rend, mouse),
  buttons_{
  Button{assets.get(TextureType::X_BUTTON), kCloseButtonRect, [&manager, this] ->  void {
      selected_tiles_.reset();
      manager.pop();
      buttons_[CONFIRM].disable();
    }},
  Button{assets.get(TextureType::ENTER_BUTTON), kConfirmButtonRect, [&playing_state, this] -> void {
      playing_state.swap_tiles(selected_tiles_);
      buttons_[CLOSE].click();
     }}
  },
  manager_{&manager}, bag_{&bag},
  remaining_text_{rend, assets.get(FontType::MOULDY_CHEESE), kFontSize, constants::kFontColorBeige},
  selected_text_{rend, assets.get(FontType::MOULDY_CHEESE), kFontSize, constants::kFontColorBeige}
{
  update_tiles_left_text();
}

void TileSwapper::ask(Texture background, std::vector<Tile> tiles) {
  background_ = std::move(background);
  SDL_SetTextureColorMod(background_.get(), constants::kDimRGB, constants::kDimRGB, constants::kDimRGB);
  tiles_ = std::move(tiles);
  update_tiles_left_text();
  update_selected_tiles_text(0);
  manager_->push(this);
  buttons_[CONFIRM].disable();
  hovered_button_ = handle_hovering(buttons_);
  hovered_tile_ = handle_hovering(tiles_);
}

void TileSwapper::update_tiles_left_text() {
  auto bag_count = static_cast<int>(bag_->tiles_left());
  if (bag_count == tiles_left_) {
    return; // no need to update if same amount
  }
  tiles_left_ = static_cast<int>(bag_->tiles_left());
  remaining_text_.update(renderer(), constants::kFontColorBeige, std::to_string(bag_count) + " tiles left", kTilesLeftPos);
}

void TileSwapper::update_selected_tiles_text(int count) {
  selected_str_[0] = static_cast<char>(count + '0');
  selected_text_.update(renderer(), constants::kFontColorBeige, selected_str_, kSelectedTilesPos);
}

void TileSwapper::render_objects() const {
  SDL_RenderCopy(renderer(), background_.get(), nullptr, nullptr);
  std::ranges::for_each(tiles_, [this](const Tile &tile) { tile.render(renderer()); });
  std::ranges::for_each( buttons_, [this](const Button &button) { button.render(renderer()); });
  remaining_text_.render(renderer());
  selected_text_.render(renderer());
}

void TileSwapper::handle_event(const SDL_Event& event) {
  static constexpr std::uint8_t kToggleHeight = 20;
  switch(event.type) {
    case SDL_MOUSEMOTION:
      set_mouse_pos({.x = event.motion.x, .y = event.motion.y});
      hovered_tile_ = handle_hovering(tiles_);
      if (hovered_tile_ == nullptr) {
        hovered_button_ = handle_hovering(buttons_);
      }
      return;
    case SDL_KEYDOWN:
      if (event.key.keysym.sym == SDLK_ESCAPE) {
        buttons_[CLOSE].click();
      } else if (event.key.keysym.sym == SDLK_RETURN and selected_tiles_.any()){
        buttons_[CONFIRM].click();
      }
      return;
    case SDL_MOUSEBUTTONUP: {
      if(hovered_tile_ != nullptr) {
        auto idx = static_cast<size_t>(std::distance(tiles_.data(), hovered_tile_));
        hovered_tile_->move({.x = hovered_tile_->x(), .y = hovered_tile_->y() + kToggleHeight - (kToggleHeight * 2) * static_cast<int>(selected_tiles_.flip(idx).test(idx))});
        int selected = static_cast<int>(selected_tiles_.count());
        update_selected_tiles_text(selected);
        selected > 0 and tiles_left_ >= selected ? buttons_[CONFIRM].enable() : buttons_[CONFIRM].disable();
      } else if(hovered_button_ != nullptr){
        click_hovered(hovered_button_);
        return;
      }
      return;
    }
    default:
      return;
  }
} 
