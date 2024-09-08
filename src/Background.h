#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "GameObject.h"
#include "constants.h"
/**
 * @class Background
 * @brief A class that represents a background object. It is the most basic type
 * of GameObject. 
 *
 */
class Background final : public GameObject {
public:
  explicit Background(SDL_Texture *texture, const SDL_Rect& rect = {0, 0, constants::kWindowWidth, constants::kWindowHeight}) : 
    GameObject(texture, rect) 
  {}

private:
  using GameObject::move;
};

#endif // BACKGROUND_H
