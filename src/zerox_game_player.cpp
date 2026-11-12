#include <zerox_input.h>

#include "zerox_game_animation.h"
#include "zerox_game_player.h"

namespace ZeroXGame
{
	void updatePlayer(Player* player, double deltaTime, const ZeroXBridge::Input* input, Animation* animations)
	{
		if ((player->currentMovementState & MovementState::JUMPING) == MovementState::JUMPING)
		{
			if ((player->previousMovementState & MovementState::JUMPING) != MovementState::JUMPING)
			{
				player->animationPlayer.timeBetweenFrames = 0.125f;
				player->animationPlayer.flags &= ~AnimationFlags::LOOP;
				playAnimation(player->gameEntity.renderEntity.material, player->animationPlayer, animations, true);
			}
		}
		else if ((player->currentMovementState & MovementState::LEFT) == MovementState::LEFT)
		{
			player->animationPlayer.flags &= ~AnimationFlags::FLIP_X;

			player->animationPlayer.timeBetweenFrames = 0.35f;
			playAnimation(player->gameEntity.renderEntity.material, player->animationPlayer, animations + 2, false);
		}
		else if ((player->currentMovementState & MovementState::RIGHT) == MovementState::RIGHT)
		{
			player->animationPlayer.flags |= AnimationFlags::FLIP_X;

			player->animationPlayer.timeBetweenFrames = 0.35f;
			playAnimation(player->gameEntity.renderEntity.material, player->animationPlayer, animations + 2, false);
		}

		if ((player->currentMovementState & MovementState::FALLING) == MovementState::FALLING)
		{
			if ((player->previousMovementState & MovementState::FALLING) != MovementState::FALLING)
			{
				player->animationPlayer.timeBetweenFrames = 0.2f;
				player->animationPlayer.flags &= ~AnimationFlags::LOOP;
				playAnimation(player->gameEntity.renderEntity.material, player->animationPlayer, animations + 1, false);
			}
		}

		updateAnimationPlayer(player->gameEntity.renderEntity.material, player->animationPlayer, deltaTime);

		player->previousMovementState = player->currentMovementState;
	}

	void fixedUpdatePlayer(Player* player, double deltaTime, const ZeroXBridge::Input* input)
	{
		// Horizontal velocity

		const float inputX = input->gamePad1.leftStickInput.x;

		player->groundLerpFactor += inputX * player->groundAcceleration * static_cast<float>(deltaTime);

		player->velocityX			= -player->maxVelocityX * (1.0f - player->groundLerpFactor) + player->maxVelocityX * player->groundLerpFactor;
		player->groundLerpFactor	= glm::clamp(player->groundLerpFactor, 0.0f, 1.0f);

		if (player->velocityX < 0.0f)
		{
			player->currentMovementState &= ~(MovementState::RIGHT | MovementState::NEUTRAL);
			player->currentMovementState |= MovementState::LEFT;
		}
		else if (player->velocityX > 0.0f)
		{
			player->currentMovementState &= ~(MovementState::LEFT | MovementState::NEUTRAL);
			player->currentMovementState |= MovementState::RIGHT;
		}
		else
		{
			player->currentMovementState &= ~(MovementState::LEFT | MovementState::RIGHT);
			player->currentMovementState |= MovementState::NEUTRAL;
		}

		// Vertical velocity

		constexpr float gravity{ -9.84f };

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

		player->gameEntity.physicsState.currentPosition.x += player->velocityX * static_cast<float>(deltaTime);
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