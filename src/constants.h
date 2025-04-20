#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SDL2/SDL_rect.h>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <functional>

namespace constants {

#ifndef NDEBUG
inline constexpr bool debug = true;
#else
inline constexpr bool debug = false;
#endif

inline constexpr const char* kGameName = "Elbbarcs";

inline constexpr uint16_t kWindowWidth{1193};
inline constexpr uint16_t kWindowHeight{895};
inline constexpr uint16_t kBoardDims{810};

inline constexpr uint8_t kTileWidth = 70;
inline constexpr uint8_t kTileHeight = 76;
inline constexpr char kTileBlankChar = ' ';
inline constexpr char kTileGapChar = '\0';

inline constexpr uint8_t kRackTileAmount = 7;

inline constexpr uint8_t kBagTileAmount = 100;

inline constexpr uint8_t kBingoScoreValue = 50;

inline constexpr std::uint8_t kNumOfTiles = 27;

inline constexpr auto kSquareNum = 15;
inline constexpr auto kTotalSquares = kSquareNum * kSquareNum;
inline constexpr auto kSquarePixelSize = kBoardDims / kSquareNum;

inline constexpr auto kRackTilePositions = std::invoke([]{
  std::array<SDL_Point, kRackTileAmount> kPositions{};
  constexpr auto kTileStartHeight = kBoardDims+5;
  constexpr auto kTileGap = kTileWidth + 5;
  constexpr auto kOffset = (kBoardDims - kTileGap*kRackTileAmount)/2;
  constexpr auto kPointGenerator = [nth_tile=0]() mutable -> SDL_Point {
    return SDL_Point{((nth_tile++)*kTileGap) + kOffset, kTileStartHeight};
  };
  std::ranges::generate(kPositions, kPointGenerator);
  return kPositions;
});

struct TileInfo {
  char letter;
  std::uint8_t frequency : 4;
  std::uint8_t value : 4;
};

inline constexpr std::array<TileInfo, kNumOfTiles> tile_info {{
      {.letter = 'a', .frequency = 9, .value = 1},
      {.letter = 'b', .frequency = 2, .value = 3},
      {.letter = 'c', .frequency = 2, .value = 3},
      {.letter = 'd', .frequency = 4, .value = 2},
      {.letter = 'e', .frequency = 12,.value = 1},
      {.letter = 'f', .frequency = 2, .value = 4},
      {.letter = 'g', .frequency = 3, .value = 2},
      {.letter = 'h', .frequency = 2, .value = 4},
      {.letter = 'i', .frequency = 9, .value = 1},
      {.letter = 'j', .frequency = 1, .value = 8},
      {.letter = 'k', .frequency = 1, .value = 5},
      {.letter = 'l', .frequency = 4, .value = 1},
      {.letter = 'm', .frequency = 2, .value = 3},
      {.letter = 'n', .frequency = 6, .value = 1},
      {.letter = 'o', .frequency = 8, .value = 1},
      {.letter = 'p', .frequency = 2, .value = 3},
      {.letter = 'q', .frequency = 1, .value = 10},
      {.letter = 'r', .frequency = 6, .value = 1},
      {.letter = 's', .frequency = 4, .value = 1},
      {.letter = 't', .frequency = 6, .value = 1},
      {.letter = 'u', .frequency = 4, .value = 1},
      {.letter = 'v', .frequency = 2, .value = 4},
      {.letter = 'w', .frequency = 2, .value = 4},
      {.letter = 'x', .frequency = 1, .value = 8},
      {.letter = 'y', .frequency = 2, .value = 4},
      {.letter = 'z', .frequency = 1, .value = 10},
      {.letter = constants::kTileBlankChar, .frequency =  2, .value = 0}, 
}};

inline constexpr std::uint32_t kRedMask = (std::endian::native == std::endian::little) ? 0x000000ff : 0xff000000;

inline constexpr std::uint32_t kGreenMask = (std::endian::native == std::endian::little) ? 0x0000ff00 : 0x00ff0000;

inline constexpr std::uint32_t kBlueMask = (std::endian::native == std::endian::little) ? 0x00ff0000 : 0x0000ff00;

inline constexpr std::uint32_t kAlphaMask = (std::endian::native == std::endian::little) ? 0xff000000 : 0x000000ff;

inline constexpr auto kDimRGB{100};

inline constexpr auto kRackButtonGap = static_cast<std::uint8_t>((kBoardDims - kRackTilePositions.back().x)/2);

inline constexpr SDL_Color kFontColorBrown{.r=54,.g=23,.b=5,.a=0};
inline constexpr SDL_Color kFontColorBeige{.r=242, .g=219, .b=145, .a=0};
static constexpr auto kHoverMod = SDL_Color{240, 235, 225, 0};
static constexpr auto kUnhoverMod = SDL_Color{255, 255, 255, 0};

} // namespace constants
#endif // CONSTANTS_H
