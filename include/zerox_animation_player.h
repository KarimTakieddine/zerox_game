#pragma once

#include <cstdint>
#include <type_traits>

#include "zerox_game_animation.h"

namespace ZeroXGame
{
	enum class AnimationFlags : size_t
	{
		NONE	= 0,
		PLAYING	= 1ULL,
		PAUSED	= 1ULL << 1,
		STOPPED	= 1ULL << 2,
		FLIP_X	= 1ULL << 3,
		FLIP_Y	= 1ULL << 4,
		LOOP	= 1ULL << 5
	};

	inline constexpr AnimationFlags operator|(AnimationFlags a, AnimationFlags b) { return static_cast<AnimationFlags>(static_cast<std::underlying_type_t<AnimationFlags>>(a) | static_cast<std::underlying_type_t<AnimationFlags>>(b)); }
	inline constexpr AnimationFlags operator&(AnimationFlags a, AnimationFlags b) { return static_cast<AnimationFlags>(static_cast<std::underlying_type_t<AnimationFlags>>(a) & static_cast<std::underlying_type_t<AnimationFlags>>(b)); }
	inline constexpr AnimationFlags& operator|=(AnimationFlags& a, AnimationFlags b) { a = a | b; return a; }
	inline constexpr AnimationFlags& operator&=(AnimationFlags& a, AnimationFlags b) { a = a & b; return a; }
	inline constexpr AnimationFlags operator~(AnimationFlags f) { return static_cast<AnimationFlags>(~static_cast<std::underlying_type_t<AnimationFlags>>(f)); }

	struct alignas(16) AnimationPlayer
	{
		const Animation* currentAnimation	{ nullptr };
		const Animation* nextAnimation		{ nullptr };
		AnimationFlags flags				{ AnimationFlags::STOPPED };
		size_t currentFrame					{ 0 };
		double timeBetweenFrames			{ 0.0f };
		double playTime						{ 0.0f };
	};

	inline constexpr bool playAnimation(ZeroXBridge::Material& material, AnimationPlayer& player, const Animation* animation, bool immediate = false)
	{
		if (animation->frameCount == 0)
		{
			return false;
		}

		if (immediate || !player.currentAnimation)
		{
			player.currentAnimation	= animation;
			player.nextAnimation	= nullptr;
			player.playTime			= 0.0;
			player.currentFrame		= 0;

			playFrame(
				material,
				animation->frames,
				(player.flags & AnimationFlags::FLIP_X) == AnimationFlags::FLIP_X,
				(player.flags & AnimationFlags::FLIP_Y) == AnimationFlags::FLIP_Y);
		}
		else
		{
			player.nextAnimation = animation;
		}

		player.flags &= ~(AnimationFlags::STOPPED | AnimationFlags::PAUSED);
		player.flags |= AnimationFlags::PLAYING;

		return true;
	}

	inline constexpr void updateAnimationPlayer(ZeroXBridge::Material& material, AnimationPlayer& player, double deltaTime)
	{
		if (!player.currentAnimation)
		{
			return;
		}

		player.playTime += deltaTime;

		if (player.playTime >= player.timeBetweenFrames)
		{
			size_t nextFrame = player.currentFrame;

			++nextFrame;

			if (nextFrame > player.currentAnimation->frameCount - 1)
			{
				if ((player.flags & AnimationFlags::LOOP) == AnimationFlags::LOOP)
				{
					playAnimation(material, player, player.currentAnimation, true);
				}
				else if (player.nextAnimation)
				{
					playAnimation(material, player, player.nextAnimation, true);
				}
				else
				{
					player.flags &= ~(AnimationFlags::PLAYING | AnimationFlags::PAUSED);
					player.flags |= AnimationFlags::STOPPED;
				}
			}
			else
			{
				playFrame(
					material,
					player.currentAnimation->frames + nextFrame,
					(player.flags & AnimationFlags::FLIP_X) == AnimationFlags::FLIP_X,
					(player.flags & AnimationFlags::FLIP_Y) == AnimationFlags::FLIP_Y);

				player.currentFrame = nextFrame;

				player.playTime = 0.0;
			}
		}
	}
}