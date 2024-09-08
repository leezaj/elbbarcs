#include "Game.h"
#include "constants.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

Game::Game() : 
  window_{SDL_CreateWindow("Scrabble", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                           constants::kWindowWidth, constants::kWindowHeight, SDL_WINDOW_RESIZABLE)},
  renderer_{SDL_CreateRenderer(window_.get(), -1, 0)},
  mouse_{{ .default_cursor = Cursor{SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW)},
           .hand_cursor = Cursor{SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND)} }},
  assets_{renderer_.get()},
  button_maker_{assets_},
  main_menu_state_{renderer_.get(),mouse_,assets_,state_manager_,button_maker_}
{
  SDL_RenderSetLogicalSize(renderer_.get(), constants::kWindowWidth, constants::kWindowHeight);
  state_manager_.push(&(main_menu_state_));
}

void Game::run() noexcept {
  #ifndef __EMSCRIPTEN__
  while(is_running_){
    process_events();
    render();
  }
  #else
    process_events();
    render();
  #endif
}

void Game::render() const {
  SDL_RenderClear(renderer_.get());
  state_manager_.top()->render_objects();
  SDL_RenderPresent(renderer_.get());
}

/* Normally, SDL_WaitEvent blocks our main thread until there is an event. We
 * then handle the event and render right after. On the web, we have to do
 * things differently because we must not block the main thread under any
 * circumstances. We thus continuously poll for events until all events are
 * handled, while still rendering after each one.

 * This type of game loop ensures no work is done, including rendering, if
 * there is no input. This in turn saves a lot of CPU cycles especially if the
 * player is idle and not doing anything.
 */
void Game::process_events() {
#ifndef __EMSCRIPTEN__
  if (SDL_WaitEvent(&event_) != 0) {
  #else
  while(SDL_PollEvent(&event_) != 0) {
  #endif
    switch(event_.type) {
      [[unlikely]] case SDL_QUIT:
        is_running_ = false;
        break;
      [[likely]] default:
        state_manager_.top()->handle_event(event_);
        break;
    }
  }
}
