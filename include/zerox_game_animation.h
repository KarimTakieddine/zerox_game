#pragma once

#include <glm/vec2.hpp>

#include <zerox_material.h>

namespace ZeroXGame
{
	struct alignas(16) AnimationFrame
	{
		glm::vec2 uvScale	{ 0.0f, 0.0f };
		glm::vec2 uvOffset	{ 0.0f, 0.0f };
	};

	struct alignas(16) Animation
	{
		AnimationFrame* frames	{ nullptr };
		size_t frameCount		{ 0 };
	};

	inline constexpr void playFrame(ZeroXBridge::Material& material, const AnimationFrame* frame, bool flipX, bool flipY)
	{
		material.uvOffset	= flipX ? glm::vec2{ frame->uvOffset.x + frame->uvScale.x, frame->uvOffset.y } : frame->uvOffset;
		material.uvScale	= flipX ? glm::vec2{ -frame->uvScale.x, frame->uvScale.y } : frame->uvScale;
	}

	/*inline constexpr void animate(ZeroXBridge::Material& material, Animation* animation, float deltaTime)
	{
		if ((animation->flags & AnimationFlags::PLAYING) != AnimationFlags::PLAYING)
		{
			return;
		}

		size_t currentFrame = animation->currentFrame;
		if (animation->currentTime >= animation->timeBetweenFrames)
		{
			if (currentFrame < animation->frameCount - 1)
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

	inline constexpr bool playAnimation(ZeroXBridge::Material& material, Animation* animation, bool immediate = false)
	{
		if (animation->frameCount == 0)
		{
			return false;
		}

		if (!immediate && ((animation->flags & AnimationFlags::PLAYING) == AnimationFlags::PLAYING))
		{
			return false;
		}

		playFrame(material, animation->frames);
		animation->currentFrame = 0;
		animation->currentTime	= 0.0f;

		animation->flags &= ~(AnimationFlags::PAUSED | AnimationFlags::STOPPED);
		animation->flags |= AnimationFlags::PLAYING;

		return true;
	}*/
}