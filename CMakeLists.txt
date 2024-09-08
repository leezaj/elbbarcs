cmake_minimum_required(VERSION 3.5.0)
project(elbbarcs VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES src/*.cc src/*.h)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 CXX_EXTENSIONS False)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unknown-warning-option -Wall -Wextra -Wpedantic -Werror -Wshadow
-pedantic -Wimplicit-fallthrough -Wconversion -Wsign-conversion -Wnon-virtual-dtor -Wold-style-cast -Wcast-align
-Wunused -Wformat=2 -Woverloaded-virtual -Wmisleading-indentation -Wduplicated-cond -Wuseless-cast -Wduplicated-branches
-Wlogical-op -Wnull-dereference -Wdouble-promotion -fno-rtti -fno-exceptions)

include(FetchContent)

FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glaze)

if(DEFINED EMSCRIPTEN)
  target_compile_options(${PROJECT_NAME} PRIVATE  --use-port=sdl2 --use-port=sdl2_image:formats=png --use-port=sdl2_ttf --use-port=sdl2_gfx --use-port=boost_headers) 
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "--use-port=sdl2 --use-port=sdl2_image:formats=png --use-port=sdl2_ttf --use-port=sdl2_gfx --use-preload-plugin --preload-file '../assets' -s -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s WASM=1  --use-preload-cache -s EVAL_CTORS=2")
  # debug linker flags
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "-g -sASSERTIONS=2 -sSTACK_OVERFLOW_CHECK=2 -fsanitize=undefined")
  set(CMAKE_EXECUTABLE_SUFFIX ".js")
else()
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/SDL2)
  find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
  find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
  find_package(Boost 1.80 REQUIRED)
  if(TARGET SDL2::SDL2main)
    target_link_libraries(${exectuable_name} SDL2::SDL2main)
  endif()
  target_link_libraries(${PROJECT_NAME} SDL2::SDL2)

  find_package(SDL2_Image REQUIRED)
  target_link_libraries(${PROJECT_NAME} SDL2_image::SDL2_image)

  find_package(SDL2_ttf REQUIRED)
  target_link_libraries(${PROJECT_NAME} SDL2_ttf::SDL2_ttf)

  find_package(SDL2_gfx REQUIRED)
  target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_GFX_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} ${SDL2_GFX_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME} glaze::glaze)
