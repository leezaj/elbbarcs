#ifndef ASSETPOOL_H
#define ASSETPOOL_H

#include "types.h"
#include <SDL2/SDL_image.h>
#include <filesystem>
#include <utility>
#include <vector>

enum class TextureType : uint8_t {
  DIALOG_BOX,
  ENTER_BUTTON,
  LOGO,
  RECALL_BUTTON,
  SCRABBLE_BOARD,
  SHUFFLE_BUTTON,
  X_BUTTON,
};

enum class FontType : uint8_t {
  LOWBALL,
  MOULDY_CHEESE,
};

template <typename T, typename... U>
concept IsAnyOf = (std::same_as<T, U> || ...);

template <typename T>
concept AssetType = IsAnyOf<T, FontType, TextureType>;

/**
 * @class AssetPool
 * @brief A class that owns different assets such as textures and fonts.
 * It provides raw non-owning pointers to the assets it owns.
 */
class AssetPool final {

public:
  explicit AssetPool(SDL_Renderer *renderer);

  /**
   * @brief Returns a non-owning raw pointer corresponding to the type provided
   */
  template <AssetType T> 
  auto *get(T type) const {
    const auto getter = [type](const auto &container) { return container[std::to_underlying(type)].get(); };
    if constexpr (std::same_as<T, TextureType>) {
      return getter(textures_);
    } else if constexpr (std::same_as<T, FontType>) {
      return getter(fonts_);
    }
  }

  static std::filesystem::path assets_path() {
    return std::filesystem::current_path().parent_path() / "assets";
  };

private:
  inline static std::filesystem::path fonts_path = assets_path()/"fonts"; 
  inline static std::filesystem::path textures_path = assets_path()/"textures";
  std::vector<Texture> textures_;
  std::vector<Font> fonts_;
};

#endif // ASSETPOOL_H
