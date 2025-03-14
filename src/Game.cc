#include "Game.h"
#include "constants.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

Game::Game() : 
  assets_{renderer_.get()},
  button_maker_{assets_},
  main_menu_state_{assets_,button_maker_}
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

void Game::render() {
  SDL_RenderClear(renderer_.get());
  state_manager_.render_current_state();
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
        utility::log("Quit signal received");
        return;
      case SDL_MOUSEMOTION:
        Mouse::pos_.x = event_.motion.x;
        Mouse::pos_.y = event_.motion.y;
        [[fallthrough]];
      default:
        state_manager_.current_state_handle_event(event_);
        break;
    }
  }
}
