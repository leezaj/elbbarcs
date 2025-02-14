cmake_minimum_required(VERSION 3.5.0)
project(elbbarcs VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES src/*.cc src/*.h)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include(FetchContent)
FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glaze)

include(FetchContent)
FetchContent_Declare(
  battery_embed
  GIT_REPOSITORY https://github.com/batterycenter/embed.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(battery_embed)

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 CXX_EXTENSIONS False)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unknown-warning-option -Wall -Wextra  -Werror -Wshadow
 -Wimplicit-fallthrough -Wconversion -Wsign-conversion -Wnon-virtual-dtor -Wold-style-cast -Wcast-align
-Wunused -Wformat=2 -Woverloaded-virtual -Wmisleading-indentation -Wduplicated-cond -Wuseless-cast -Wduplicated-branches
-Wlogical-op -Wnull-dereference -Wdouble-promotion -fno-rtti -fno-exceptions)
target_compile_definitions(${PROJECT_NAME} PUBLIC B_PRODUCTION_MODE=ON)

if(NOT DEFINED EMSCRIPTEN)
  #we need to do this, otherwise it complains about strings from our embedded binaries being too long
  target_compile_options(${PROJECT_NAME} PRIVATE -Wpedantic -pedantic) 
endif()

b_embed(${PROJECT_NAME} assets/dict.bin)
b_embed(${PROJECT_NAME} assets/tiles/a.png)
b_embed(${PROJECT_NAME} assets/tiles/b.png)
b_embed(${PROJECT_NAME} assets/tiles/blank_tile.png)
b_embed(${PROJECT_NAME} assets/tiles/c.png)
b_embed(${PROJECT_NAME} assets/tiles/d.png)
b_embed(${PROJECT_NAME} assets/tiles/e.png)
b_embed(${PROJECT_NAME} assets/tiles/f.png)
b_embed(${PROJECT_NAME} assets/tiles/g.png)
b_embed(${PROJECT_NAME} assets/tiles/h.png)
b_embed(${PROJECT_NAME} assets/tiles/i.png)
b_embed(${PROJECT_NAME} assets/tiles/j.png)
b_embed(${PROJECT_NAME} assets/tiles/k.png)
b_embed(${PROJECT_NAME} assets/tiles/l.png)
b_embed(${PROJECT_NAME} assets/tiles/m.png)
b_embed(${PROJECT_NAME} assets/tiles/n.png)
b_embed(${PROJECT_NAME} assets/tiles/o.png)
b_embed(${PROJECT_NAME} assets/tiles/p.png)
b_embed(${PROJECT_NAME} assets/tiles/q.png)
b_embed(${PROJECT_NAME} assets/tiles/r.png)
b_embed(${PROJECT_NAME} assets/tiles/s.png)
b_embed(${PROJECT_NAME} assets/tiles/t.png)
b_embed(${PROJECT_NAME} assets/tiles/u.png)
b_embed(${PROJECT_NAME} assets/tiles/v.png)
b_embed(${PROJECT_NAME} assets/tiles/w.png)
b_embed(${PROJECT_NAME} assets/tiles/x.png)
b_embed(${PROJECT_NAME} assets/tiles/y.png)
b_embed(${PROJECT_NAME} assets/tiles/z.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/a.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/b.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/c.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/d.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/e.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/f.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/g.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/h.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/i.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/j.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/k.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/l.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/m.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/n.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/o.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/p.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/q.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/r.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/s.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/t.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/u.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/v.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/w.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/x.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/y.png)
b_embed(${PROJECT_NAME} assets/blank-tiles/z.png)
b_embed(${PROJECT_NAME} assets/textures/board.png)
b_embed(${PROJECT_NAME} assets/textures/dialog_box.png)
b_embed(${PROJECT_NAME} assets/textures/enter_beige.png)
b_embed(${PROJECT_NAME} assets/textures/logo.png)
b_embed(${PROJECT_NAME} assets/textures/recall_beige.png)
b_embed(${PROJECT_NAME} assets/textures/shuffle_beige.png)
b_embed(${PROJECT_NAME} assets/textures/x_beige.png)
b_embed(${PROJECT_NAME} assets/fonts/LowballNeueRegular-rglJB.ttf)
b_embed(${PROJECT_NAME} assets/fonts/MouldyCheeseRegular-WyMWG.ttf)
b_embed(${PROJECT_NAME} assets/templates/wood_button.png)


if(DEFINED EMSCRIPTEN)
  target_compile_options(${PROJECT_NAME} PRIVATE  --use-port=sdl2 --use-port=sdl2_image:formats=png --use-port=sdl2_ttf --use-port=sdl2_gfx)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "--use-port=sdl2 --use-port=sdl2_image:formats=png --use-port=sdl2_ttf --use-port=sdl2_gfx -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s WASM=1  --use-preload-cache -s EVAL_CTORS=2")
  # debug linker flags
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "-g -sASSERTIONS=2 -sSTACK_OVERFLOW_CHECK=2 -fsanitize=undefined")
  set(CMAKE_EXECUTABLE_SUFFIX ".js")
else()
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/SDL2)
  find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
  find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
  if(TARGET SDL2::SDL2main)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main)
  endif()
  target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

  find_package(SDL2_Image REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_image::SDL2_image)

  find_package(SDL2_ttf REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_ttf::SDL2_ttf)

  find_package(SDL2_gfx REQUIRED)
  target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_GFX_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_GFX_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)
