#pragma once

#include <cstdint>

#include "zerox_game_constants.h"
#include "zerox_game_memory_view.h"

namespace ZeroXGame
{
	template<typename T>
	inline constexpr std::span<T> readContiguousArray(unsigned char* start, uint64_t count)
	{
		return { reinterpret_cast<T*>(start), static_cast<size_t>(count) };
	}

	GameMemoryView readGameMemory(unsigned char* data);
	LiveMemoryView readLiveMemory(unsigned char* data);
}
