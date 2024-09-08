#ifndef TEXT_H
#define TEXT_H

#include "types.h"
#include <SDL2/SDL_ttf.h>
#include <string_view>

class Text final {
public:
  Text(SDL_Renderer *renderer, TTF_Font *font, int font_size, SDL_Color color, std::string_view content, SDL_Point position);

  Text(SDL_Renderer *renderer, TTF_Font* font, int font_size, SDL_Color color);

  void center_text(int area_start_x, int area_width, int y_pos);

  void render(SDL_Renderer* renderer) const;

  void update(SDL_Renderer *renderer, SDL_Color color, std::string_view content, SDL_Point position);

  void update(SDL_Renderer *renderer, SDL_Color color, std::string_view content);

  void update(SDL_Renderer* renderer, std::string_view content);

  [[nodiscard]] std::int32_t width() const {return rect_.w;}

  [[nodiscard]] std::int32_t height() const {return rect_.h;}

  [[nodiscard]] std::int32_t x() const {return rect_.x;}

  [[nodiscard]] std::int32_t y() const {return rect_.y;}

  [[nodiscard]] SDL_Rect rect() const {return rect_;}

  Texture take_texture() {return std::move(texture_);}

  void move(SDL_Point point) {rect_.x = point.x; rect_.y = point.y;}

private:
  void make_rect(SDL_Point position, std::string_view content);

  TTF_Font* font_;
  Texture texture_;
  int font_size_;
  SDL_Rect rect_;
  SDL_Color color_;
};

#endif // TEXT_H
