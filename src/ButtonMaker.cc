#include "ButtonMaker.h"
#include "constants.h"

namespace {
const auto btn_path = AssetPool::assets_path()/"templates"/"wood_button.png";
} // namespace

ButtonMaker::ButtonMaker(const AssetPool& assets):
  font_{assets.get(FontType::MOULDY_CHEESE)},
  wood_button_{IMG_Load(btn_path.c_str())}
{}

Button ButtonMaker::make_text_button(SDL_Renderer *rend, std::string_view text,
                                     const SDL_Rect &position,
                                     std::function<void()> callback) {
  static const auto kBtnH = wood_button_->h, kBtnW = wood_button_->w;
  const Surface copy{SDL_CreateRGBSurface(0, kBtnW, kBtnH, 32, 
      constants::kRedMask, constants::kGreenMask, constants::kBlueMask, constants::kAlphaMask)};
  SDL_BlitSurface(wood_button_.get(), nullptr, copy.get(), nullptr);
  TTF_SetFontSize(font_, kBtnH/2);
  const Surface text_surface{TTF_RenderUTF8_Blended(font_, text.data(), constants::kFontColorBrown)};
  // the shorter a word is, the greater the horizontal padding
  const std::int32_t horiz_padding = (std::max(0, 10 - static_cast<int>(text.size()))) * 4 + kBtnH / 4;
  // the rectangle that the text will be in when 'pasted' onto the button
  SDL_Rect text_rect{.x = horiz_padding, .y = 2*kBtnH / 7, .w = (kBtnW) - (2 * horiz_padding), .h = (2 * kBtnH) / 4};
  SDL_BlitScaled(text_surface.get(), nullptr, copy.get(), &text_rect);
  button_textures_.emplace_back(SDL_CreateTextureFromSurface(rend, copy.get()));
  return Button{button_textures_.back().get(), position, std::move(callback)};
}
