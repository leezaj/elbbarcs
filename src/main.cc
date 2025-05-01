#include "Game.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

void game_loop(void* arg) {
  static_cast<Game*>(arg)->run();
}
#endif

int main() {
  utility::log("Starting {}", constants::kGameName);
  Game game;
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(&game_loop, &game, 0, true);
#else
  game.run();
#endif // __EMSCRIPTEN__
}
