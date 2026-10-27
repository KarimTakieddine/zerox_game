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

        const size_t backgroundCount{ 2 };
        allocator->requestMemory<uint64_t>(backgroundCount);
        auto* backgrounds = allocator->requestMemoryArray<Background>(backgroundCount);

        for (size_t i = 0; i < backgroundCount; ++i)
        {
            (backgrounds + i)->layer = static_cast<uint64_t>(i);
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

        auto* backgroundEntitySection = allocator->requestMemory<EntitySection>(
            EntitySection{
                .type   = EntitySectionType::BACKGROUND,
                .flags  = EntitySectionFlags::ALLOCATE_BATCH
            }
        );

        ++(*entitySectionCount);

        const size_t spriteCount    = playerCount + enemyCount;
        auto* spriteEntitySection   = allocator->requestMemory<EntitySection>(EntitySection{
            .type   = EntitySectionType::SPRITE,
            .flags  = EntitySectionFlags::ALLOCATE_BATCH });

        ++(*entitySectionCount);

        auto* colliderEntitySection = allocator->requestMemory<EntitySection>(EntitySection{
            .type   = EntitySectionType::COLLIDER,
            .flags  = EntitySectionFlags::ALLOCATE_BATCH });

        ++(*entitySectionCount);

        auto* backgroundReadDescriptors = allocator->requestMemoryArray<EntityReadDescriptor>(backgroundCount);
        auto* spriteReadDescriptors     = allocator->requestMemoryArray<EntityReadDescriptor>(spriteCount);
        auto* colliderReadDescriptors   = allocator->requestMemoryArray<EntityReadDescriptor>(colliderCount);

        // Set entity section offsets

        MemoryCursor<16> entityCursor(sizeof(ZeroXBridge::Input));

        entityCursor.step<uint64_t>();
        for (size_t i = 0; i < backgroundCount; ++i)
        {
            *(backgroundReadDescriptors + i) = { .start = entityCursor.getOffset() };

            entityCursor.step<Background>();
        }

        backgroundEntitySection->readDescriptors = backgroundReadDescriptors;
        backgroundEntitySection->descriptorCount = backgroundCount;

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
        static double physicsAccumulator{ 0.0 };

        GameMemory gameMemory = readGameMemory(allocator);
        
        const auto backgrounds = gameMemory.backgrounds;
        for (size_t i = 0; i < backgrounds.size(); ++i)
        {
            updateBackground(*(backgrounds.data() + i));
        }

        physicsAccumulator += deltaTime;

        const auto enemies = gameMemory.enemies;
        
        float fixedDeltaTime = 1.0 / 90;
        while (physicsAccumulator >= fixedDeltaTime)
        {
            for (size_t i = 0; i < enemies.size(); ++i)
            {
                auto* enemy = (enemies.data() + i);

                enemy->physicsState.lastPosition        = enemy->physicsState.currentPosition;
                enemy->physicsState.currentPosition.x   += (i + 1) * 0.90f * fixedDeltaTime;
            }

            physicsAccumulator -= fixedDeltaTime;
        }

        const float alpha = static_cast<float>(physicsAccumulator / fixedDeltaTime);

        for (size_t i = 0; i < enemies.size(); ++i)
        {
            auto* enemy = (enemies.data() + i);

            const glm::vec2 lerpPosition =
                enemy->physicsState.lastPosition * (1.0f - alpha) +
                enemy->physicsState.currentPosition * alpha;

            setPosition(enemy->transform, lerpPosition);
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