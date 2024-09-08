#include "Text.h"
Text::Text(SDL_Renderer *renderer, TTF_Font *font, int font_size, SDL_Color color, std::string_view content, SDL_Point position) : 
  font_{font},
  font_size_{font_size},
  color_{color}
{
  update(renderer, color, content, position);
}

Text::Text(SDL_Renderer *renderer, TTF_Font *font, int font_size, SDL_Color color) : 
  Text(renderer, font, font_size, color, std::string_view{}, SDL_Point{}) 
{}


void Text::render(SDL_Renderer* renderer) const {
  SDL_RenderCopy(renderer, texture_.get(), nullptr, &rect_);
}

void Text::make_rect(SDL_Point position, std::string_view content) {
  TTF_SizeUTF8(font_, content.data(), &rect_.w, &rect_.h);
  rect_.x = position.x;
  rect_.y = position.y;
}

void Text::update(SDL_Renderer *renderer, SDL_Color color, std::string_view content, SDL_Point position) {
  if(content.empty())
    content = "\0";
  TTF_SetFontSize(font_, font_size_);
  Surface temp{TTF_RenderUTF8_Blended(font_, content.data(), color)};
  texture_ = Texture{SDL_CreateTextureFromSurface(renderer, temp.get())};
  make_rect(position, content);
}

void Text::update(SDL_Renderer *renderer, SDL_Color color, std::string_view content) {
  update(renderer, color, content, {rect_.x, rect_.y});
}

void Text::update(SDL_Renderer *renderer, std::string_view content) {
  update(renderer, color_, content);
}

void Text::center_text(int area_start_x, int area_width, int y_pos){
  rect_.x = std::max(area_start_x, area_start_x + (area_width - rect_.w)/2);
  rect_.y = y_pos;
}
