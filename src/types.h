/**
 * @file types.h
 * @brief Definitions for various types used in the program.
 */

#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include <stack>
#include <vector>

/**
 * @brief A struct that is templated with the non-type template
 * paramater Function. The struct's operator() takes a pointer argument and
 * calls Function on that argument. This is useful because SDL uses custom
 * deleters for various things and we want std::unique_ptr to use them (and
 * std::unique_ptr<T,D> requires the deleter D to be a type.)
 *
 * @param obj A pointer that should be freed.
 */
template <auto Function> struct Wrapper final {
  static void operator()(auto* obj) { Function(obj); }
};

class GameState;

using Cursor = std::unique_ptr<SDL_Cursor, Wrapper<SDL_FreeCursor>>;
using Font = std::unique_ptr<TTF_Font, Wrapper<TTF_CloseFont>>;
using Renderer = std::unique_ptr<SDL_Renderer, Wrapper<SDL_DestroyRenderer>>;
using Surface = std::unique_ptr<SDL_Surface, Wrapper<SDL_FreeSurface>>;
using Texture = std::unique_ptr<SDL_Texture, Wrapper<SDL_DestroyTexture>>;
using Window = std::unique_ptr<SDL_Window, Wrapper<SDL_DestroyWindow>>;

using GameStateManager = std::stack<GameState*, std::vector<GameState*>>;

#endif // TYPES_H
