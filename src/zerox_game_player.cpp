#include <zerox_input.h>

#include "zerox_game_animation.h"
#include "zerox_game_player.h"

namespace ZeroXGame
{
	void updatePlayer(Player* player, double deltaTime, const ZeroXBridge::Input* input, Animation* animations)
	{
		if (ZeroXBridge::isButtonPressedThisFrame(input->gamePad1, ZeroXBridge::Button::BUTTON_A))
		{
			playAnimation(player->gameEntity.renderEntity.material, animations);
		}

		if ((player->currentMovementState & MovementState::FALLING) == MovementState::FALLING)
		{
			if ((player->previousMovementState & MovementState::FALLING) != MovementState::FALLING)
			{
				playAnimation(player->gameEntity.renderEntity.material, animations + 1);

				player->previousMovementState |= MovementState::FALLING;
			}
		}

		animate(player->gameEntity.renderEntity.material, animations, static_cast<float>(deltaTime));
		animate(player->gameEntity.renderEntity.material, animations + 1, static_cast<float>(deltaTime));

		/*if ((player->currentMovementState & MovementState::FALLING) == MovementState::FALLING)
		{
			if ((player->previousMovementState & MovementState::FALLING) != MovementState::FALLING)
			{
				playAnimation(player->gameEntity.renderEntity.material, animations + 1);
			}
		}
		else if (ZeroXBridge::isButtonPressedThisFrame(input->gamePad1, ZeroXBridge::Button::BUTTON_A))
		{
			playAnimation(player->gameEntity.renderEntity.material, animations + 1);
		}

		animate(player->gameEntity.renderEntity.material, animations, static_cast<float>(deltaTime));

		player->currentMovementState = player->previousMovementState;*/

		/*if (ZeroXBridge::isButtonPressedThisFrame(input->gamePad1, ZeroXBridge::Button::BUTTON_A))
		{
			playAnimation(player->gameEntity.renderEntity.material, animations + 1);
		}

		animate(player->gameEntity.renderEntity.material, animations, static_cast<float>(deltaTime));*/
	}

	void fixedUpdatePlayer(Player* player, double deltaTime, const ZeroXBridge::Input* input)
	{
		constexpr float gravity = { -9.84f };

		float velocityY = 0.0f;

		if (ZeroXBridge::isButtonPressedThisFrame(input->gamePad1, ZeroXBridge::Button::BUTTON_A))
		{
			if ((player->currentMovementState & MovementState::JUMPING) != MovementState::JUMPING)
			{
				player->initialJumpVelocity		= 12.0f;
				player->jumpTimer				= 0.0f;
				player->currentMovementState	&= ~MovementState::FALLING;
				player->previousMovementState	&= ~MovementState::FALLING;
				player->currentMovementState	|= MovementState::JUMPING;
			}
		}

		if ((player->currentMovementState & MovementState::JUMPING) == MovementState::JUMPING)
		{
			player->jumpTimer += 3.5f * static_cast<float>(deltaTime);
		}

		velocityY = gravity * 0.5f * player->jumpTimer + player->initialJumpVelocity;

		if (velocityY < 0.0f)
		{
			player->currentMovementState |= MovementState::FALLING;
		}

		player->gameEntity.physicsState.currentPosition.y += velocityY * static_cast<float>(deltaTime);
	}
}