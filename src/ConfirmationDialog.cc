#include "ConfirmationDialog.h"
#include "Game.h"
#include "Playing.h"
#include <SDL2/SDL_render.h>
#include <algorithm>

namespace {
constexpr SDL_Rect kDialogBoxRect{.x = 200, .y = 200, .w = 750, .h = 425};
constexpr SDL_Rect kConfirmButtonRect{.x = 350,.y = 450, .w = 180, .h = 96};
constexpr SDL_Rect kDeclineButtonRect{.x = 600, .y = 450, .w = 180, .h = 96};
constexpr std::uint8_t kPromptFontSize = 60;
} // namespace

ConfirmationDialog::ConfirmationDialog(Playing& playing_state, const AssetPool& assets, ButtonMaker& button_maker) :
  dialog_box_{assets.get(TextureType::DIALOG_BOX)},
  prompt_{Game::renderer(), assets.get(FontType::MOULDY_CHEESE), kPromptFontSize, constants::kFontColorBrown},
  buttons_{
    Button{button_maker.make_text_button(Game::renderer(), "Confirm", kConfirmButtonRect, []{})}, 
    Button{button_maker.make_text_button(Game::renderer(), "Decline", kDeclineButtonRect, &Game::pop_game_state)}
  },
  playing_state_{&playing_state}
{
  utility::log("Confirmation dialog initialized");
}
  
void ConfirmationDialog::render_objects() const {
  SDL_RenderCopy(Game::renderer(), background_.get(), nullptr, nullptr);
  SDL_RenderCopy(Game::renderer(), dialog_box_, nullptr, &kDialogBoxRect);
  std::ranges::for_each( buttons_, [](const Button &button) static { button.render(Game::renderer());});
  prompt_.render(Game::renderer());
}

void ConfirmationDialog::ask(std::string_view prompt, std::function<void()> confirm_action) {
  static constexpr std::int32_t kBoxLeftSide = 275;
  static constexpr std::int32_t kBoxWidth = 600;
  static constexpr std::int32_t kPromptYPos = 350;
  prompt_.update(Game::renderer(), prompt);
  prompt_.center_text(kBoxLeftSide, kBoxWidth, kPromptYPos);
  background_ = playing_state_->get_snapshot();
  SDL_SetTextureColorMod(background_.get(), constants::kDimRGB, constants::kDimRGB, constants::kDimRGB);
  buttons_[CONFIRM].set_click([func = std::move(confirm_action)] { Game::pop_game_state(); func(); });
  Game::push_game_state(this);
  hovered_button_ = Mouse::handle_hovering(buttons_);
}

void ConfirmationDialog::handle_event(const SDL_Event& event) {
  switch(event.type) {
  case SDL_MOUSEMOTION:
    hovered_button_ = Mouse::handle_hovering(buttons_);
    return;
  case SDL_MOUSEBUTTONUP:
    Mouse::click_hovered(hovered_button_);
    return;
  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_ESCAPE) {
      buttons_[DECLINE].click();
    } else if (event.key.keysym.sym == SDLK_RETURN) {
      buttons_[CONFIRM].click();
    }
    return;
  default:
    return;
  }
}
