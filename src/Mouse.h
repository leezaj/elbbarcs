#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_rect.h>

struct Cursors final { Cursor default_cursor; Cursor hand_cursor; };

class Mouse final {
public:
  explicit Mouse(Cursors cursors):
    hand_cursor_{std::move(cursors.hand_cursor)},
    default_cursor_{std::move(cursors.default_cursor)}
  {}

  [[nodiscard]] SDL_Point pos() const {return pos_;}

  void set_pos(SDL_Point pos) {pos_ = pos;}

  void set_default_cursor() const {SDL_SetCursor(default_cursor_.get());}

  void set_hand_cursor() const {SDL_SetCursor(hand_cursor_.get());}
private:
  Cursor hand_cursor_, default_cursor_;
  SDL_Point pos_{};
};

#endif // MOUSE_H
