#ifndef GAME_H
#define GAME_H
#include "ButtonMaker.h"
#include "AssetPool.h"
#include "MainMenu.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>

/**
 * @class Game
 * @brief Class representing the game and everything in it.
 *
 */
class Game final {
public:
  /**
   * @brief Load the game and all of its contents.
   */
  Game();

  /**
   * @brief Run the game.
   */
  void run() noexcept;

private:
  void process_events();

  void render() const;

  struct SDL_RAII final {
    SDL_RAII() {
      SDL_SetMainReady();
      SDL_VideoInit(nullptr);
      TTF_Init();
      SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
    }
    ~SDL_RAII() {
      TTF_Quit();
      SDL_VideoQuit();
      SDL_Quit();
    }
    SDL_RAII(const SDL_RAII &) = delete;
    SDL_RAII(SDL_RAII &&) = delete;
    SDL_RAII &operator=(const SDL_RAII &) = delete;
    SDL_RAII &operator=(SDL_RAII &&) = delete;
  } sdl_init_;
  bool is_running_{true};
  Window window_;
  Renderer renderer_;
  Mouse mouse_;
  AssetPool assets_;
  ButtonMaker button_maker_;
  MainMenu main_menu_state_;
  SDL_Event event_{};
  GameStateManager state_manager_;
};

#endif // GAME_H
