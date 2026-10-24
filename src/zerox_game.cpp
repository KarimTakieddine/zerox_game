#include <zerox_render_descriptor.h>

#include "zerox_game.h"
#include "zerox_game_entity.h"
#include "zerox_game_memory.hpp"

namespace ZeroXGame
{
    using ZeroXBridge::EntitySection;
    using ZeroXBridge::EntitySectionType;
    using ZeroXBridge::EntitySectionFlags;
    using ZeroXBridge::EntityReadDescriptor;
    using ZeroXBridge::RenderDescriptor;

    bool initializeGame(GameAllocator* allocator)
    {
        if (!allocator)
        {
            return false;
        }

        const size_t playerCount{ 1 };
        allocator->requestMemory<uint64_t>(playerCount);
        auto* players = allocator->requestMemoryArray<GameEntity>(playerCount);

        for (size_t i = 0; i < playerCount; ++i)
        {
            (players + i)->material.uvScale = { 0.0625f, 0.0625f };
        }

        const size_t enemyCount{ 4 };
        allocator->requestMemory<uint64_t>(enemyCount);
        auto* enemies = allocator->requestMemoryArray<GameEntity>(enemyCount);

        for (size_t i = 0; i < enemyCount; ++i)
        {
            auto* enemy = enemies + i;

            enemy->material.uvScale         = { 0.0625f, 0.0625f };
            enemy->material.textureOffset   = { 0.0f, 0.0625f };
        }

        const size_t spriteCount = playerCount + enemyCount;
        if (spriteCount == 0)
        {
            return true;
        }

        auto* entitySectionCount    = allocator->requestMemory<uint64_t>();
        *entitySectionCount         = 1;

        auto* spriteEntitySection = allocator->requestMemory<EntitySection>(EntitySection{
            .type   = EntitySectionType::SPRITE,
            .flags  = EntitySectionFlags::ALLOCATE_BATCH | EntitySectionFlags::SET_VERTEX_LAYOUT });

        auto* spriteReadDescriptors = allocator->requestMemoryArray<EntityReadDescriptor>(spriteCount);

        MemoryCursor<16> entityCursor(sizeof(ZeroXBridge::Input));

        entityCursor.step<uint64_t>();
        for (size_t i = 0; i < playerCount; ++i)
        {
            *(spriteReadDescriptors + i) = { .start = entityCursor.getOffset() };

            entityCursor.step<GameEntity>();
        }

        entityCursor.step<uint64_t>();
        for (size_t i = 0; i < enemyCount; ++i)
        {
            *(spriteReadDescriptors + playerCount + i) = { .start = entityCursor.getOffset() };

            entityCursor.step<GameEntity>();
        }

        spriteEntitySection->readDescriptors    = spriteReadDescriptors;
        spriteEntitySection->descriptorCount    = spriteCount;

        return true;
    }

    void updateGame(GameAllocator* allocator, double deltaTime)
    {
        GameMemory gameMemory = readGameMemory(allocator);
        
        const auto players = gameMemory.players;
        for (size_t i = 0; i < players.size(); ++i)
        {
            auto* player = players.data() + i;

            player->transform.localToWorld[3].x += 0.35f * deltaTime;
        }

        const auto enemies = gameMemory.enemies;
        for (size_t i = 0; i < enemies.size(); ++i)
        {
            auto* enemy = enemies.data() + i;

            enemy->transform.localToWorld[3].x += (i + 1) * 0.25f * deltaTime;
        }
    }

    RenderDescriptor initializeRenderDescriptor(const GameAllocator* allocator)
    {
        GameMemory gameMemory = readGameMemory(allocator);
        
        const size_t entitySectionCount = *( gameMemory.entitySectionCount.data() );

        if (entitySectionCount == 0)
        {
            return { };
        }

        return {
            .entitySections     = reinterpret_cast<EntitySection*>(gameMemory.entitySectionSpan.data()),
            .entitySectionCount = entitySectionCount
        };
    }
}