#include <zerox_input.h>
#include <zerox_render_descriptor.h>

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
		uint64_t nextOffset = *offset;

		const auto* memoryHeader = ::readMemoryHeader(start + nextOffset).data();

		nextOffset += sizeof(GameMemoryHeader);

		const uint64_t count = memoryHeader->count;

		auto result = ZeroXGame::readContiguousArray<T>(start + nextOffset, count);
		nextOffset += ( count * sizeof(T) );

		*offset = nextOffset;

		return result;
	}
}

namespace ZeroXGame
{
	GameMemoryView readGameMemory(unsigned char* data)
	{
		uint64_t offset{ 0 };

		return {
			.input		= ::readMemoryBlock<ZeroXBridge::Input>(data, &offset),
			.levels		= ::readMemoryBlock<LevelView>(data, &offset),
			.animations	= ::readMemoryBlock<Animation>(data, &offset)
		};
	}

	LiveMemoryView readLiveMemory(unsigned char* data)
	{
		size_t offset{ 0 };

		return {
			.level				= ::readMemoryBlock<LevelView>(data, &offset),
			.colliderEntities	= ::readMemoryBlock<Collider2DEntity>(data, &offset),
			.entitySections		= ::readMemoryBlock<ZeroXBridge::EntitySection>(data, &offset)
		};
	}
}