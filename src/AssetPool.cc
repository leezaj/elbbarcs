#include "AssetPool.h"
#include <algorithm>
#include <functional>

namespace {

Texture to_texture(SDL_Renderer *rend, const std::filesystem::path &path) {
  return Texture{IMG_LoadTexture(rend, path.c_str())};
}

Font to_font(const std::filesystem::path &path) {
  return Font{TTF_OpenFont(path.c_str(), 0)};
}

template <typename T>
auto load_assets(const std::filesystem::path &path, T transform_func) {
  // determine the type of the asset based on the return type of the transform
  using Asset = std::invoke_result_t<decltype(transform_func), decltype(path)>;
  std::vector<std::filesystem::path> files{std::filesystem::directory_iterator(path), {}};
  std::ranges::sort(files); // needed for consistent ordering
  std::vector<Asset> assets;
  assets.reserve(files.size());
  std::ranges::transform(files, std::back_inserter(assets), transform_func);
  return assets;
}

} // namespace

AssetPool::AssetPool(SDL_Renderer* renderer) : 
  textures_{load_assets(textures_path, std::bind_front(to_texture, renderer))},
  fonts_{load_assets(fonts_path, to_font)}
{}
