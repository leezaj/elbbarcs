#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SDL2/SDL_rect.h>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <functional>

namespace constants {
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
    return SDL_Point{(nth_tile++)*kTileGap + kOffset, kTileStartHeight};
  };
  std::ranges::generate(kPositions, kPointGenerator);
  return kPositions;
});

inline constexpr std::uint32_t kRedMask = (std::endian::native == std::endian::little) ? 0x000000ff : 0xff000000;

inline constexpr std::uint32_t kGreenMask = (std::endian::native == std::endian::little) ? 0x0000ff00 : 0x00ff0000;

inline constexpr std::uint32_t kBlueMask = (std::endian::native == std::endian::little) ? 0x00ff0000 : 0x0000ff00;

inline constexpr std::uint32_t kAlphaMask = (std::endian::native == std::endian::little) ? 0xff000000 : 0x000000ff;

inline constexpr auto kDimRGB{100};

inline constexpr uint8_t kRackButtonGap = (kBoardDims - kRackTilePositions.back().x) / 2;

inline constexpr SDL_Color kFontColorBrown{.r=54,.g=23,.b=5,.a=0};
inline constexpr SDL_Color kFontColorBeige{.r=242, .g=219, .b=145, .a=0};

} // namespace constants
#endif // CONSTANTS_H
