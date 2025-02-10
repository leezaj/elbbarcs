#ifndef ASSETPOOL_H
#define ASSETPOOL_H

#include "types.h"
#include <concepts>
#include "battery/embed.hpp"
#include <SDL2/SDL_image.h>
#include <utility>
#include <vector>
#include "utility.h"

enum class TextureType : uint8_t {
  BOARD,
  DIALOG_BOX,
  ENTER_BUTTON,
  LOGO,
  RECALL_BUTTON,
  SHUFFLE_BUTTON,
  X_BUTTON,
};

enum class FontType : uint8_t {
  LOWBALL,
  MOULDY_CHEESE,
};

/**
 * @class AssetPool
 * @brief A class that owns different assets such as textures and fonts.
 * It provides raw non-owning pointers to the assets it owns.
 */
class AssetPool final {

private:
  using File = b::EmbedInternal::EmbeddedFile;

  static constexpr Font to_font(const File& file) {
    SDL_RWops* buffer = SDL_RWFromConstMem(file.data(), static_cast<int>(file.size()));
    return Font{TTF_OpenFontRW(buffer, 1, 0)}; // buffer freed by '1' parameter
  }

  static constexpr Texture to_texture(SDL_Renderer* renderer, const File& file) {
    SDL_RWops* buffer = SDL_RWFromConstMem(file.data(), static_cast<int>(file.size()));
    return Texture{IMG_LoadTexture_RW(renderer, buffer, 1)}; // buffer freed by '1' parameter
  }

public:
  constexpr AssetPool(SDL_Renderer* renderer) : 
    textures_{utility::map(std::to_array({
      b::embed<"assets/textures/board.png">(),
      b::embed<"assets/textures/dialog_box.png">(),
      b::embed<"assets/textures/enter_beige.png">(),
      b::embed<"assets/textures/logo.png">(),
      b::embed<"assets/textures/recall_beige.png">(),
      b::embed<"assets/textures/shuffle_beige.png">(),
      b::embed<"assets/textures/x_beige.png">()}), std::bind_front(AssetPool::to_texture, renderer))},
    fonts_{utility::map(std::to_array({
      b::embed<"assets/fonts/LowballNeueRegular-rglJB.ttf">(),
      b::embed<"assets/fonts/MouldyCheeseRegular-WyMWG.ttf">()}), AssetPool::to_font)}
  {}

  /**
   * @brief Returns a non-owning raw pointer corresponding to the type provided
   */
  template <typename T> requires std::same_as<T, TextureType> || std::same_as<T, FontType>
  constexpr auto *get(T type) const {
    const auto getter = [type](const auto &container) { return container[std::to_underlying(type)].get(); };
    if constexpr (std::same_as<T, TextureType>) {
      return getter(textures_);
    } else if constexpr (std::same_as<T, FontType>) {
      return getter(fonts_);
    }
  }

private:
  std::vector<Texture> textures_;
  std::vector<Font> fonts_;
};

#endif // ASSETPOOL_H
