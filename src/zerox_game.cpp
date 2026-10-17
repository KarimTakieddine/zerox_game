#include "zerox_game.h"
#include "zerox_game_entity.h"
#include "zerox_game_memory.hpp"

namespace ZeroXGame
{
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

        auto* spriteEntitySection = allocator->requestMemory<ZeroX::EntitySection>(ZeroX::EntitySection{
            .type   = ZeroX::EntitySectionType::SPRITE,
            .flags  = ZeroX::EntitySectionFlags::ALLOCATE_BATCH | ZeroX::EntitySectionFlags::SET_VERTEX_LAYOUT });

        auto* spriteFetchDescriptors = allocator->requestMemoryArray<ZeroX::EntityFetchDescriptor>(spriteCount);

        MemoryCursor<16> entityCursor(sizeof(ZeroX::Input));

        entityCursor.step<uint64_t>();
        for (size_t i = 0; i < playerCount; ++i)
        {
            *(spriteFetchDescriptors + i) = {
                .stride = 0,
                .offset = entityCursor.getOffset()
            };

            entityCursor.step<GameEntity>();
        }

        entityCursor.step<uint64_t>();
        for (size_t i = 0; i < enemyCount; ++i)
        {
            *(spriteFetchDescriptors + playerCount + i) = {
                .stride = 0,
                .offset = entityCursor.getOffset()
            };

            entityCursor.step<GameEntity>();
        }

        spriteEntitySection->fetchDescriptors   = spriteFetchDescriptors;
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

            player->transform.localToWorld[3].x -= 0.35f * deltaTime;
        }

        const auto enemies = gameMemory.enemies;
        for (size_t i = 0; i < enemies.size(); ++i)
        {
            auto* enemy = enemies.data() + i;

            enemy->transform.localToWorld[3].x -= (i + 1) * 0.25f * deltaTime;
        }
    }

    ZeroX::RenderDescriptor initializeRenderDescriptor(const GameAllocator* allocator)
    {
        ZeroX::RenderDescriptor result = { };

        GameMemory gameMemory = readGameMemory(allocator);
        
        const size_t entitySectionCount = *( gameMemory.entitySectionCount.data() );

        if (entitySectionCount == 0)
        {
            return result;
        }

        result.sectionCount         = entitySectionCount;
        result.entitySectionData    = gameMemory.entitySectionSpan.data();

        return result;
    }
}