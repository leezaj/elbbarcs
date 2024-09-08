#ifndef BUTTONMAKER_H
#define BUTTONMAKER_H

#include "AssetPool.h"
#include "Button.h"
#include "types.h"

class ButtonMaker final {
public:
  explicit ButtonMaker(const AssetPool& assets);

  /**
   * @brief Creates a wooden button. The ButtonMaker will own the texture that the Button internally points to.
   */
  [[nodiscard]] Button make_text_button(SDL_Renderer *rend, std::string_view text, const SDL_Rect &position, std::function<void()> callback);

private:
  TTF_Font* font_;
  std::vector<Texture> button_textures_;
  Surface wood_button_;
};

#endif
