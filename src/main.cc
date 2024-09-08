#include "Game.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

void game_loop(void* arg) {
  static_cast<Game*>(arg)->run();
}
#endif

int main() {
  Game game;
  #ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(&game_loop, &game, -1, true);
  #else
  game.run();
  #endif
}
