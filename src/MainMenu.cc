#include "types.h"
#include "Game.h"
#include "MainMenu.h"
#include "Mouse.h"

namespace {

constexpr SDL_Rect kBoardBgSrc{
  .x = 0,
  .y = 0,
  .w = constants::kBoardDims - 1,
  .h = constants::kBoardDims - 1};

constexpr SDL_Rect kBoardBgDest{
  .x = (constants::kWindowWidth / 2) - ((constants::kBoardDims - 1) / 2),
  .y = (constants::kWindowHeight / 2) - ((constants::kBoardDims - 1) / 2),
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
  .x = (constants::kWindowWidth/2) - (335/2),
  .y = 250,
  .w = 335,
  .h = 60
};

constexpr int kFontSize = 50, kOptionBegin = 425, kOptionGap = 110, kAngleRotate = 180;

} // namespace

MainMenu::MainMenu(const AssetPool &assets, ButtonMaker& button_maker):
  MainMenu{assets, {{"Play", "Credits"}}, {{ 
      // "Play" button action
      [&, this]() {
        // Lazy load the playing state
        if (not playing_state_) {
          utility::log("Initializing playing state...");
          playing_state_.emplace(assets, button_maker);
        } else {
          playing_state_->restart_game();
        }
        Game::push_game_state(&(*playing_state_));
      },
      // "Credits" button action
      std::bind_front(SDL_OpenURL, "https://github.com/leezaj/elbbarcs#credits")
    }} 
  }
{}


MainMenu::MainMenu(const AssetPool &assets, 
                   std::array<std::string_view, MainMenu::NUM_BUTTONS> texts, 
                   std::array<std::function<void()>, MainMenu::NUM_BUTTONS> callbacks) :
  background_{assets.get(TextureType::BOARD)},
  logo_{assets.get(TextureType::LOGO)},
  buttons_{
    utility::map<utility::to_array>(texts, [&assets, &callbacks, idx = 0](std::string_view text) mutable {
      Text temp{Game::renderer(), assets.get(FontType::MOULDY_CHEESE), kFontSize, constants::kColorBrown, text, {}};
      temp.center_text(kBoardBgDest.x, kBoardBgDest.w, kOptionBegin + (kOptionGap * idx));
      Texture texture = temp.take_texture();
      return std::pair<Button, Texture>(std::piecewise_construct,
          std::forward_as_tuple(texture.get(), temp.rect(), std::move(callbacks[static_cast<size_t>(idx++)])), 
          std::forward_as_tuple(std::move(texture))
      );
    })
  }
{
  utility::log("Main menu initialized");
}

void MainMenu::render_objects() const {
  SDL_RenderCopy(Game::renderer(), background_, &kBoardBgSrc, &kBoardBgDest);
  SDL_RenderCopy(Game::renderer(), background_, &kHorizontalSrc, &kRightDest);
  SDL_RenderCopy(Game::renderer(), background_, &kHorizontalSrc, &kLeftDest);
  SDL_RenderCopy(Game::renderer(), background_, &kVerticalSrc, &kUpDest);
  SDL_RenderCopyEx(Game::renderer(), background_, &kVerticalSrc, &kDownDest, kAngleRotate, nullptr, SDL_FLIP_VERTICAL);
  SDL_RenderCopy(Game::renderer(), logo_, nullptr, &kLogoRect);
  std::ranges::for_each(buttons_ | std::views::keys, [](const auto& btn) static { btn.render(Game::renderer());});
}

void MainMenu::handle_event(const SDL_Event& event) {
  switch(event.type) {
    case SDL_MOUSEMOTION:
      hovered_button_ = Mouse::handle_hovering(buttons_ | std::views::keys);
      return;
    case SDL_MOUSEBUTTONUP:
      Mouse::click_hovered(hovered_button_);
      hovered_button_ = nullptr;
      return;
    default:
      return;
  }
}
