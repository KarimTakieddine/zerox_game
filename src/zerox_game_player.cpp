#include <zerox_input.h>

#include "zerox_game_animation.h"
#include "zerox_game_player.h"

namespace ZeroXGame
{
	void updatePlayer(Player* player, double deltaTime, const ZeroXBridge::Input* input, Animation* animations)
	{
		if (ZeroXBridge::isButtonPressedThisFrame(input->gamePad1, ZeroXBridge::Button::BUTTON_A))
		{
			player->animationPlayer.timeBetweenFrames = 0.125f;
			playAnimation(player->gameEntity.renderEntity.material, player->animationPlayer, animations, true);
		}

		if ((player->currentMovementState & MovementState::FALLING) == MovementState::FALLING)
		{
			if ((player->previousMovementState & MovementState::FALLING) != MovementState::FALLING)
			{
				player->animationPlayer.timeBetweenFrames = 0.25f;
				playAnimation(player->gameEntity.renderEntity.material, player->animationPlayer, animations + 1, true);

				player->previousMovementState |= MovementState::FALLING;
			}
		}

		updateAnimationPlayer(player->gameEntity.renderEntity.material, player->animationPlayer, deltaTime);
	}

	void fixedUpdatePlayer(Player* player, double deltaTime, const ZeroXBridge::Input* input)
	{
		constexpr float gravity = { -9.84f };

		float velocityX = 0.0f;

		velocityX = input->gamePad1.leftStickInput.x * 10.0f;

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

		player->gameEntity.physicsState.currentPosition.x += velocityX * static_cast<float>(deltaTime);
		player->gameEntity.physicsState.currentPosition.y += velocityY * static_cast<float>(deltaTime);
	}

	void onCollisionEnter(Player* player, const Collider* collider)
	{
		if (collider->normal.y > 0.0f)
		{
			player->currentMovementState	&= ~(MovementState::FALLING|MovementState::JUMPING);
			player->initialJumpVelocity		= 0.0f;
			player->jumpTimer				= 0.0f;
		}
	}

	void onCollisionExit(Player* player, const Collider* collider)
	{
		
	}
}