#include <zerox_input.h>

#include "zerox_game_memory_header.h"
#include "zerox_game_memory_reader.hpp"

namespace
{
	using ZeroXGame::GameMemoryHeader;

	constexpr std::span<const GameMemoryHeader, 1> readMemoryHeader(const unsigned char* start)
	{
		return std::span<const GameMemoryHeader, 1>{ reinterpret_cast<const GameMemoryHeader*>(start), 1 };
	}

	template<typename T>
	constexpr std::span<T> readMemoryBlock(unsigned char* start, uint64_t* offset)
	{
		const auto* memoryHeader = ::readMemoryHeader(start).data();

		*offset += sizeof(GameMemoryHeader);
		start	+= *offset;

		const uint64_t count = memoryHeader->count;

		auto result = ZeroXGame::readContiguousArray<T>(start, count);
		*offset		+= ( count * sizeof(T) );

		return result;
	}
}

namespace ZeroXGame
{
	GameMemoryView readGameMemory(const GameAllocator* allocator)
	{
		GameMemoryView gameMemoryView;

		unsigned char* start = reinterpret_cast<unsigned char*>(allocator->peek());
		size_t offset{ 0 };

		gameMemoryView.input	= readMemoryBlock<ZeroXBridge::Input>(start, &offset);
		gameMemoryView.levels	= readMemoryBlock<LevelView>(start, &offset);

		return gameMemoryView;
	}
}