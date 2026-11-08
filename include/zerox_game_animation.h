#pragma once

#include <cstdint>
#include <type_traits>

#include <glm/vec2.hpp>

#include <zerox_material.h>

namespace ZeroXGame
{
	struct alignas(16) AnimationFrame
	{
		glm::vec2 uvScale	{ 0.0f, 0.0f };
		glm::vec2 uvOffset	{ 0.0f, 0.0f };
	};

	enum class AnimationFlags : uint64_t
	{
		NONE	= 0,
		STOPPED = 1ULL,
		PLAYING	= 1ULL << 1,
		PAUSED	= 1ULL << 2,
		LOOP	= 1ULL << 3,
		FLIP_X	= 1ULL << 4,
		FLIP_Y	= 1ULL << 5
	};

	inline constexpr AnimationFlags operator|(AnimationFlags a, AnimationFlags b) { return static_cast<AnimationFlags>(static_cast<std::underlying_type_t<AnimationFlags>>(a) | static_cast<std::underlying_type_t<AnimationFlags>>(b)); }
	inline constexpr AnimationFlags operator&(AnimationFlags a, AnimationFlags b) { return static_cast<AnimationFlags>(static_cast<std::underlying_type_t<AnimationFlags>>(a) & static_cast<std::underlying_type_t<AnimationFlags>>(b)); }
	inline constexpr AnimationFlags& operator|=(AnimationFlags& a, AnimationFlags b) { a = a | b; return a; }
	inline constexpr AnimationFlags& operator&=(AnimationFlags& a, AnimationFlags b) { a = a & b; return a; }
	inline constexpr AnimationFlags operator~(AnimationFlags f) { return static_cast<AnimationFlags>(~static_cast<std::underlying_type_t<AnimationFlags>>(f)); }

	struct alignas(16) Animation
	{
		AnimationFlags flags	{ AnimationFlags::NONE };
		const char* name		{ nullptr };
		AnimationFrame* frames	{ nullptr };
		size_t frameCount		{ 0 };
		size_t currentFrame		{ 0 };
		float timeBetweenFrames	{ 0.0f };
		float currentTime		{ 0.0f };
	};

	inline constexpr void playFrame(ZeroXBridge::Material& material, const AnimationFrame* frame)
	{
		material.uvOffset	= frame->uvOffset;
		material.uvScale	= frame->uvScale;
	}

	inline constexpr void animate(ZeroXBridge::Material& material, Animation* animation, float deltaTime)
	{
		if ((animation->flags & AnimationFlags::PLAYING) != AnimationFlags::PLAYING)
		{
			return;
		}

		size_t currentFrame = animation->currentFrame;
		if (animation->currentTime >= animation->timeBetweenFrames)
		{
			if (currentFrame < animation->frameCount)
			{
				++currentFrame;
				playFrame(material, animation->frames + currentFrame);
			}
			else if ((animation->flags & AnimationFlags::LOOP) == AnimationFlags::LOOP)
			{
				currentFrame = 0;
				playFrame(material, animation->frames + currentFrame);
				
			}
			else
			{
				animation->flags &= ~AnimationFlags::PLAYING;
				animation->flags |= AnimationFlags::STOPPED;
			}

			animation->currentTime = -deltaTime;
		}

		animation->currentFrame = currentFrame;
		animation->currentTime += deltaTime;
	}

	inline constexpr bool playAnimation(ZeroXBridge::Material& material, Animation* animation)
	{
		if (animation->frameCount == 0)
		{
			return false;
		}

		playFrame(material, animation->frames);
		animation->currentFrame = 0;
		animation->currentTime	= 0.0f;

		animation->flags &= ~(AnimationFlags::PAUSED | AnimationFlags::STOPPED);
		animation->flags |= AnimationFlags::PLAYING;
	}
}