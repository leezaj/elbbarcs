#ifndef GAME_H
#define GAME_H
#include "ButtonMaker.h"
#include "AssetPool.h"
#include "MainMenu.h"
#include "GameStateManager.h"
#include "Mouse.h"
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

  Game();

  void run() noexcept;

  static SDL_Renderer* renderer() {return renderer_.get();}

  static void push_game_state(GameState auto* state) {state_manager_.push(state);}

  static void pop_game_state() {state_manager_.pop();}

  template <GameState T>
  static void pop_until() {
    state_manager_.pop_until<T>();
  }

private:
  void process_events();

  static void render();

  struct SDL_RAII final {
    constexpr SDL_RAII() {
      SDL_SetMainReady();
      SDL_VideoInit(nullptr);
      TTF_Init();
      SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
    }
    constexpr ~SDL_RAII() {
      TTF_Quit();
      SDL_VideoQuit();
      SDL_Quit();
    }
    SDL_RAII(const SDL_RAII &) = delete;
    SDL_RAII(SDL_RAII &&) = delete;
    SDL_RAII &operator=(const SDL_RAII &) = delete;
    SDL_RAII &operator=(SDL_RAII &&) = delete;
  };

  inline static SDL_RAII sdl_init_{};
  inline static constinit bool is_running_{true};
  inline static Window window_{SDL_CreateWindow(
      constants::kGameName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      constants::kWindowWidth, constants::kWindowHeight, SDL_WINDOW_RESIZABLE)};
  inline static Renderer renderer_{SDL_CreateRenderer(window_.get(), -1, 0)};
  inline static Mouse mouse_{};
  inline static constinit GameStateManager<MainMenu, Playing, ConfirmationDialog, TileSwapper, BlankTileReplacer, GameOver> state_manager_{};
  AssetPool assets_;
  ButtonMaker button_maker_;
  MainMenu main_menu_state_;
  SDL_Event event_{};
};

#endif // GAME_H
