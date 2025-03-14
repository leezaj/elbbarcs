#ifndef ASSETPOOL_H
#define ASSETPOOL_H

#include "battery/embed.hpp"
#include "types.h"
#include <SDL2/SDL_image.h>
#include <utility>
#include "utility.h"

enum class TextureType : uint8_t {
  BOARD,
  DIALOG_BOX,
  ENTER_BUTTON,
  LOGO,
  RECALL_BUTTON,
  SHUFFLE_BUTTON,
  X_BUTTON,
  MAX
};

enum class FontType : uint8_t {
  LOWBALL,
  MOULDY_CHEESE,
  MAX
};


/**
 * @class AssetPool
 * @brief A class that owns different assets such as textures and fonts.
 * It provides raw non-owning pointers to the assets it owns.
 */
class AssetPool final {

private:
public:
  explicit AssetPool(SDL_Renderer* renderer) : 
    textures_{utility::map<utility::to_array>(textures, std::bind_front(AssetPool::to_texture, renderer))},
    fonts_{utility::map<utility::to_array>(fonts, AssetPool::to_font)}
  {
    utility::log("Asset pool initialized");
  }

  template <typename T> requires std::same_as<T, TextureType> || std::same_as<T, FontType>
  constexpr auto *get(T type) const {
    if constexpr (std::same_as<T, TextureType>) {
      return textures_[std::to_underlying(type)].get();
    } else if constexpr (std::same_as<T, FontType>) {
      return fonts_[std::to_underlying(type)].get();
    }
  }

private:
  using File = b::EmbedInternal::EmbeddedFile;

  static constexpr Font to_font(const File& file) {
    SDL_RWops* buffer = SDL_RWFromConstMem(file.data(), static_cast<int>(file.size()));
    return Font{TTF_OpenFontRW(buffer, 1, 0)}; // buffer freed by '1' parameter
  }

  static constexpr Texture to_texture(SDL_Renderer* renderer, const File& file) {
    RWops buffer{SDL_RWFromConstMem(file.data(), static_cast<int>(file.size()))};
    Surface surface{IMG_LoadWEBP_RW(buffer.get())};
    return Texture{SDL_CreateTextureFromSurface(renderer, surface.get())};
  }

  static constexpr std::array textures{
      b::embed<"assets/textures/board.webp">(),
      b::embed<"assets/textures/dialog_box.webp">(),
      b::embed<"assets/textures/enter_beige.webp">(),
      b::embed<"assets/textures/logo.webp">(),
      b::embed<"assets/textures/recall_beige.webp">(),
      b::embed<"assets/textures/shuffle_beige.webp">(),
      b::embed<"assets/textures/x_beige.webp">(),
};

  static constexpr std::array fonts {
    b::embed<"assets/fonts/LowballNeue-Regular.ttf">(),
    b::embed<"assets/fonts/MouldyCheese-Regular.ttf">(), 
};

  std::array<Texture, static_cast<size_t>(TextureType::MAX)> textures_;
  std::array<Font, static_cast<size_t>(FontType::MAX)> fonts_;
};

#endif // ASSETPOOL_H
