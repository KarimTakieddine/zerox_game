#include <zerox_render_descriptor.h>

#include "zerox_game.h"
#include "zerox_game_entity.h"
#include "zerox_game_memory.hpp"
#include "zerox_game_updater.h"

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

        // Allocate game entities

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

        const size_t colliderCount{ 5 };
        allocator->requestMemory<uint64_t>(colliderCount);
        auto* colliders = allocator->requestMemoryArray<Collider>(colliderCount);

        for (size_t i = 0; i < colliderCount; ++i)
        {
            auto* collider = colliders + i;

            if (i < playerCount)
            {
                collider->target = &(players + i)->transform;
            }
            else if (i - playerCount < enemyCount)
            {
                collider->target = &(enemies + i - playerCount)->transform;
            }
            else
            {
                collider->target = nullptr;
            }
        }

        // Allocate entity sections

        auto* entitySectionCount = allocator->requestMemory<uint64_t>(0);

        const size_t spriteCount    = playerCount + enemyCount;
        auto* spriteEntitySection   = allocator->requestMemory<EntitySection>(EntitySection{
            .type   = EntitySectionType::SPRITE,
            .flags  = EntitySectionFlags::ALLOCATE_BATCH });

        ++(*entitySectionCount);

        auto* colliderEntitySection = allocator->requestMemory<EntitySection>(EntitySection{
            .type   = EntitySectionType::COLLIDER,
            .flags  = EntitySectionFlags::ALLOCATE_BATCH });

        ++(*entitySectionCount);

        auto* spriteReadDescriptors     = allocator->requestMemoryArray<EntityReadDescriptor>(spriteCount);
        auto* colliderReadDescriptors   = allocator->requestMemoryArray<EntityReadDescriptor>(colliderCount);

        // Set entity section offsets

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

        spriteEntitySection->readDescriptors = spriteReadDescriptors;
        spriteEntitySection->descriptorCount = spriteCount;

        entityCursor.step<uint64_t>();
        for (size_t i = 0; i < colliderCount; ++i)
        {
            *(colliderReadDescriptors + i) = { .start = entityCursor.getOffset() };

            entityCursor.step<Collider>();
        }

        colliderEntitySection->readDescriptors = colliderReadDescriptors;
        colliderEntitySection->descriptorCount = colliderCount;

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

        const auto colliders = gameMemory.colliders;
        for (size_t i = 0; i < colliders.size(); ++i)
        {
            updateCollider(*(colliders.data() + i));
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