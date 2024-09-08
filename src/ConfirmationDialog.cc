#include "ConfirmationDialog.h"
#include "Playing.h"
#include <SDL2/SDL_render.h>
#include <algorithm>

constexpr SDL_Rect kDialogBoxRect{.x = 200, .y = 200, .w = 750, .h = 425};
constexpr SDL_Rect kConfirmButtonRect{.x = 350,.y = 450, .w = 180, .h = 96};
constexpr SDL_Rect kDeclineButtonRect{.x = 600, .y = 450, .w = 180, .h = 96};
constexpr std::uint8_t kPromptFontSize = 60;

namespace {
  enum Buttons : std::uint8_t {CONFIRM, DECLINE};
} // namespace

ConfirmationDialog::ConfirmationDialog(SDL_Renderer* rend, Mouse& mouse, GameStateManager& manager, 
                                       Playing& playing_state, const AssetPool& assets, ButtonMaker& button_maker) :
  GameState(rend, mouse),
  dialog_box_{assets.get(TextureType::DIALOG_BOX), kDialogBoxRect},
  prompt_{rend, assets.get(FontType::MOULDY_CHEESE), kPromptFontSize, constants::kFontColorBrown},
  buttons_ {
    Button {button_maker.make_text_button(rend, "Confirm", kConfirmButtonRect, []{})}, 
    Button{button_maker.make_text_button(rend, "Decline", kDeclineButtonRect, [&manager]{manager.pop();})}
  },
  state_manager_{&manager},
  playing_state_{&playing_state}
{}
  
void ConfirmationDialog::render_objects() const {
  SDL_RenderCopy(renderer(), background_.get(), nullptr, nullptr);
  dialog_box_.render(renderer());
  std::ranges::for_each( buttons_, [this](const Button &button) { button.render(renderer());});
  prompt_.render(renderer());
}

void ConfirmationDialog::ask(std::string_view prompt, std::function<void()> confirm_action) {
  static constexpr std::int32_t kBoxLeftSide = 275;
  static constexpr std::int32_t kBoxWidth = 600;
  static constexpr std::int32_t kPromptYPos = 350;
  prompt_.update(renderer(), prompt);
  prompt_.center_text(kBoxLeftSide, kBoxWidth, kPromptYPos);
  background_ = playing_state_->get_snapshot();
  SDL_SetTextureColorMod(background_.get(), constants::kDimRGB, constants::kDimRGB, constants::kDimRGB);
  buttons_[CONFIRM].set_click([this, func = std::move(confirm_action)] { state_manager_->pop(); func(); });
  state_manager_->push(this);
  hovered_button_ = handle_hovering(buttons_);
}

void ConfirmationDialog::handle_event(const SDL_Event& event) {
  switch(event.type) {
  case SDL_MOUSEMOTION:
    set_mouse_pos({.x = event.motion.x, .y=event.motion.y});
    hovered_button_ = handle_hovering(buttons_);
    return;
  case SDL_MOUSEBUTTONUP:
    click_hovered(hovered_button_);
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
