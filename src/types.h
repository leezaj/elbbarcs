#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include <utility>

template <auto Function> struct Wrapper final {
  static void operator()(auto* obj) { Function(obj); }
};

using Cursor = std::unique_ptr<SDL_Cursor, Wrapper<SDL_FreeCursor>>;
using Font = std::unique_ptr<TTF_Font, Wrapper<TTF_CloseFont>>;
using Renderer = std::unique_ptr<SDL_Renderer, Wrapper<SDL_DestroyRenderer>>;
using Surface = std::unique_ptr<SDL_Surface, Wrapper<SDL_FreeSurface>>;
using Texture = std::unique_ptr<SDL_Texture, Wrapper<SDL_DestroyTexture>>;
using Window = std::unique_ptr<SDL_Window, Wrapper<SDL_DestroyWindow>>;
using RWops = std::unique_ptr<SDL_RWops, Wrapper<SDL_RWclose>>;

template <typename T>
concept Rectangle = std::same_as<std::remove_cvref_t<T>, SDL_Rect> || std::same_as<std::remove_cvref_t<T>, SDL_FRect>;

template <typename T>
concept Hoverable = requires(const T object) {
  {object.hover()} -> std::same_as<void>;
  {object.unhover()} -> std::same_as<void>;
} && (requires(const T object) {
    {object.rect} -> Rectangle;
  } || requires(const T object) {
    {object.rectangle()} -> Rectangle;
  });

template <typename T>
concept GameState  = requires(T state, SDL_Event event) {
  {std::as_const(state).render_objects()} -> std::same_as<void>;
  {state.handle_event(event)} -> std::same_as<void>;
};

#endif // TYPES_H
