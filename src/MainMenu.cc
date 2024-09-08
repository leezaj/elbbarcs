#include "MainMenu.h"

namespace {

enum Buttons : std::uint8_t {PLAY, CREDITS, NUM_BUTTONS};

constexpr SDL_Rect kBoardBgSrc{
  .x = 0,
  .y = 0,
  .w = constants::kBoardDims - 1,
  .h = constants::kBoardDims - 1};

constexpr SDL_Rect kBoardBgDest{
  .x = constants::kWindowWidth / 2 - (constants::kBoardDims - 1) / 2,
  .y = constants::kWindowHeight / 2 - (constants::kBoardDims - 1) / 2,
  .w = constants::kBoardDims - 1,
  .h = constants::kBoardDims - 1};

constexpr SDL_Rect kHorizontalSrc{
  .x = constants::kBoardDims + 1,
  .y = constants::kBoardDims + 1,
  .w = constants::kWindowWidth - constants::kBoardDims + 1,
  .h = constants::kWindowHeight - constants::kBoardDims + 1};

constexpr SDL_Rect kRightDest{
  .x = kBoardBgDest.x + kBoardBgDest.w + 1,
  .y = 0,
  .w = kBoardBgDest.x,
  .h = constants::kWindowHeight};

constexpr SDL_Rect kLeftDest{
  .x = 0,
  .y = 0,
  .w = kBoardBgDest.x,
  .h = constants::kWindowHeight
};

constexpr SDL_Rect kVerticalSrc{
  .x = 0,
  .y = constants::kBoardDims+1,
  .w = constants::kBoardDims,
  .h = constants::kWindowHeight - (constants::kBoardDims+1)
};

constexpr SDL_Rect kUpDest{
  .x = kLeftDest.w,
  .y = 0,
  .w = constants::kBoardDims,
  .h = (constants::kWindowHeight - kBoardBgDest.h)/2
};

constexpr SDL_Rect kDownDest{
  .x = kLeftDest.w,
  .y = kBoardBgDest.y + kBoardBgDest.h,
  .w = constants::kBoardDims,
  .h = (constants::kWindowHeight - kBoardBgDest.h)/2
};

constexpr SDL_Rect kLogoRect {
  .x = constants::kWindowWidth/2 - 335/2,
  .y = 250,
  .w = 335,
  .h = 60
};

constexpr int kFontSize = 50, kOptionBegin = 425, kOptionGap = 110, kAngleRotate = 180;

} // namespace

MainMenu::MainMenu(SDL_Renderer *rend, Mouse& mouse, const AssetPool &assets, GameStateManager& manager, ButtonMaker& button_maker):
  GameState{rend, mouse},
  logo_{assets.get(TextureType::LOGO), kLogoRect},
  background_{assets.get(TextureType::SCRABBLE_BOARD)}
{
  std::array<const char *, NUM_BUTTONS> texts{"Play", "Credits"};
  std::array<std::function<void()>, NUM_BUTTONS> callbacks{
      // "Play" button action
      [&, rend, this]() {
        // Lazy load the playing state
        if (not playing_state_) {
          playing_state_.emplace(rend, mouse, assets, manager, button_maker);
        } else {
          playing_state_->restart_game();
        }
        manager.push(&(*playing_state_));
      },
      // "Credits" button action
      [] { SDL_OpenURL("https://github.com/leezaj/elbbarcs#credits"); }};
  for (size_t i = 0; i < NUM_BUTTONS; ++i) {
    Text temp{rend, assets.get(FontType::MOULDY_CHEESE), kFontSize, constants::kFontColorBrown, texts[i],  {}};
    temp.center_text(kBoardBgDest.x, kBoardBgDest.w, kOptionBegin + kOptionGap * static_cast<int>(i));
    button_textures_.emplace_back(temp.take_texture());
    buttons_.emplace_back(button_textures_.back().get(), temp.rect(), std::move(callbacks[i]));
  }
}

void MainMenu::render_objects() const {
  SDL_RenderCopy(renderer(), background_, &kBoardBgSrc, &kBoardBgDest);
  SDL_RenderCopy(renderer(), background_, &kHorizontalSrc, &kRightDest);
  SDL_RenderCopy(renderer(), background_, &kHorizontalSrc, &kLeftDest);
  SDL_RenderCopy(renderer(), background_, &kVerticalSrc, &kUpDest);
  SDL_RenderCopyEx(renderer(), background_, &kVerticalSrc, &kDownDest, kAngleRotate, nullptr, SDL_FLIP_VERTICAL);
  logo_.render(renderer());
  std::ranges::for_each(buttons_, [this](const auto& btn){ btn.render(renderer());});
}

void MainMenu::handle_event(const SDL_Event& event) {
  switch(event.type) {
    case SDL_MOUSEMOTION:
      set_mouse_pos({.x = event.motion.x, .y = event.motion.y});
      hovered_button_ = handle_hovering(buttons_);
      return;
    case SDL_MOUSEBUTTONUP:
      click_hovered(hovered_button_);
  }
}
