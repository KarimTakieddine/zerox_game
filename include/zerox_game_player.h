#pragma once

#include <cstdint>
#include <type_traits>

#include "zerox_animation_player.h"
#include "zerox_game_collider.h"

namespace ZeroXBridge { struct Input; }

namespace ZeroXGame
{
	struct Animation;

	enum class MovementState : unsigned
	{
		NONE	= 0,
		NEUTRAL = 1,
		LEFT	= 1 << 1,
		RIGHT	= 1 << 2,
		JUMPING	= 1 << 3,
		FALLING	= 1 << 4
	};

	inline constexpr MovementState operator|(MovementState a, MovementState b) { return static_cast<MovementState>(static_cast<std::underlying_type_t<MovementState>>(a) | static_cast<std::underlying_type_t<MovementState>>(b)); }
	inline constexpr MovementState operator&(MovementState a, MovementState b) { return static_cast<MovementState>(static_cast<std::underlying_type_t<MovementState>>(a) & static_cast<std::underlying_type_t<MovementState>>(b)); }
	inline constexpr MovementState& operator|=(MovementState& a, MovementState b) { a = a | b; return a; }
	inline constexpr MovementState& operator&=(MovementState& a, MovementState b) { a = a & b; return a; }
	inline constexpr MovementState operator~(MovementState m) { return static_cast<MovementState>(~static_cast<std::underlying_type_t<MovementState>>(m)); }

	struct alignas(16) Player
	{
		GameEntity gameEntity;
		AnimationPlayer animationPlayer;
		MovementState currentMovementState	{ MovementState::NEUTRAL|MovementState::FALLING };
		MovementState previousMovementState	{ MovementState::NONE };
		float initialJumpVelocity			{ 0.0f };
		float jumpTimer						{ 2.0f };
		float groundAcceleration			{ 0.5f };
		float groundDecceleration			{ 1.0f };
		float groundLerpFactor				{ 0.5f };
		float airLerpFactor					{ 0.0f };
		float velocityX						{ 0.0f };
		float maxVelocityX					{ 5.0f };
		float fallGravityFactor				{ 1.0f };
		float minVelocityY					{ -10.0f };
	};

	void updatePlayer(Player* player, double deltaTime, const ZeroXBridge::Input* input, Animation* animations);
	void fixedUpdatePlayer(Player* player, double deltaTime, const ZeroXBridge::Input* input);
	void onCollisionEnter(Player* player, const Collider* collider);
	void onCollisionExit(Player* player, const Collider* collider);
}
