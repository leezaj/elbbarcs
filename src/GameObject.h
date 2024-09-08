#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

class GameObject {
protected:
  GameObject(SDL_Texture *texture, const SDL_Rect& rect) : 
    rect_{rect}, texture_{texture} 
  {};
public:
  struct Dimensions { int w; int h; };

  void render(SDL_Renderer* renderer) const {
    SDL_RenderCopy(renderer, texture_, nullptr, &rect_);
  };

  [[nodiscard]] SDL_Point point() const { return {.x = rect_.x, .y = rect_.y}; }
  
  [[nodiscard]] int x() const {return rect_.x;}
  
  [[nodiscard]] int y() const{return rect_.y;}

  [[nodiscard]] const auto& rectangle() const { return rect_; }
  
  void move(SDL_Point point) noexcept {
    rect_.x = point.x;
    rect_.y = point.y;
  }
  
  void set_dimensions(Dimensions dim) {
    rect_.w = dim.w;
    rect_.h = dim.h;
  }
  
  [[nodiscard]] SDL_Texture* texture() const { return texture_; }

private:
  SDL_Rect rect_;
  SDL_Texture* texture_;
};

#endif // GAMEOBJECT_H
