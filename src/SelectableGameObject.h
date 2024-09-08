#ifndef SELECTABLEGAMEOBJECT_H
#define SELECTABLEGAMEOBJECT_H

#include "GameObject.h"
class SelectableGameObject: public GameObject {
protected:
  SelectableGameObject(SDL_Texture* texture, SDL_Rect rect):
    GameObject(texture, rect)
  {}
public:
  void hover() const {
    SDL_SetTextureColorMod(texture(), 240, 235, 225);
  }
  
  void unhover() const {
    SDL_SetTextureColorMod(texture(), 255, 255, 255);
  }
};

#endif // SELECTABLEGAMEOBJECT_H
