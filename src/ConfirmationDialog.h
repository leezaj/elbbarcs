#ifndef CONFIRMATIONDIALOG_H
#define CONFIRMATIONDIALOG_H

#include "ButtonMaker.h"
#include "Background.h"
#include "Button.h"
#include "GameState.h"
#include "Text.h"
#include <string_view>

class Playing;

/**
 * @class ConfirmationDialog
 * @brief A game state that presents a confirmation dialog. A prompt and a
 * callback must be given. When prompted, clicking 'decline'
 * will exit the game state, while clicking 'confirm' will invoke a function
 * before exiting the game state.
 *
 */
class ConfirmationDialog final : public GameState {
public:
  /**
   * @brief Create a confirmation dialog game state. It won't prompt anything unless ask() is called.
   */
  ConfirmationDialog(SDL_Renderer *rend, Mouse &mouse, GameStateManager &manager, Playing &playing_state,
                     const AssetPool &assets, ButtonMaker &button_maker);

  /**
   * @copydoc GameState::render_objects()
   */
  void render_objects() const final;

  /**
   * @copydoc GameState::handle_event(const SDL_Event&)
   */
  void handle_event(const SDL_Event& event) final;

  /**
   * @brief Prompt the player whether they want to do something. The two buttons
   * avialable to press will be "Confirm" and "Decline". Decline does nothing
   * except exit out of the dialog, while clicking Confirm will invoke the
   * provided callback and then exit.
   *
   * @param prompt the text that will be displayed in the center
   * @param confirm_action the function to call if "Confirm" is clicked
   */
  void ask(std::string_view prompt, std::function<void()> confirm_action);
private:
  Background dialog_box_;
  Texture background_;
  Text prompt_;
  std::array<Button, 2> buttons_;
  const Button *hovered_button_{};
  GameStateManager* state_manager_;
  Playing* playing_state_;
};

#endif // CONFIRMATIONDIALOG_H
