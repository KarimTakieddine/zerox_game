#include "zerox_game.h"
#include "zerox_game_logic.h"

#include <zerox_level.hpp>

namespace ZeroXGame
{
    using ZeroXBridge::EntitySection;
    using ZeroXBridge::EntitySectionType;
    using ZeroXBridge::EntitySectionFlags;
    using ZeroXBridge::EntityReadDescriptor;
    using ZeroXBridge::RenderDescriptor;

    ZeroXBridge::GameContext initializeGame(GameAllocator* allocator)
    {
        ZeroXBridge::GameContext context = { .input = allocator->requestMemory<ZeroXBridge::Input>() };

        if (!allocator)
        {
            return context;
        }

        const auto levelCount   = *allocator->requestMemory<uint64_t>(1);
        auto* levels            = allocator->requestMemoryArray<ZeroXBridge::Level>(1);

        const auto backgroundCount  = *allocator->requestMemory<uint64_t>(2);
        auto* backgroundConfigs     = allocator->requestMemoryArray<ZeroXBridge::BackgroundConfig>(backgroundCount);

        backgroundConfigs[0].transformConfig.position   = { 0.0f, 0.0f };
        backgroundConfigs[0].transformConfig.scale      = { 33.0f, 16.875f };

        backgroundConfigs[1].transformConfig.position   = { 0.0f, 0.0f };
        backgroundConfigs[1].transformConfig.scale      = { 33.0f, 16.875f };

        levels->backgrounds     = backgroundConfigs;
        levels->backgroundCount = backgroundCount;

        const auto playerCount  = *allocator->requestMemory<uint64_t>(1);
        auto* playerConfigs     = allocator->requestMemoryArray<ZeroXBridge::PlayerConfig>(playerCount);

        playerConfigs[0].transformConfig.position   = { 0.0f, 0.0f };
        playerConfigs[0].materialConfig.uvScale     = { 0.0625f, 0.0625f };

        levels->players     = playerConfigs;
        levels->playerCount = playerCount;

        const auto enemyCount   = *allocator->requestMemory<uint64_t>(4);
        auto* enemyConfigs      = allocator->requestMemoryArray<ZeroXBridge::EnemyConfig>(enemyCount);

        enemyConfigs[0].transformConfig.position    = { 3.0f, 0.0f };
        enemyConfigs[0].materialConfig.uvScale      = { 0.0625f, 0.0625f };
        enemyConfigs[0].materialConfig.uvOffset     = { 0.0f, 0.0625f };

        enemyConfigs[1].transformConfig.position    = { 6.0f, 0.0f };
        enemyConfigs[1].materialConfig.uvScale      = { 0.0625f, 0.0625f };
        enemyConfigs[1].materialConfig.uvOffset     = { 0.0f, 0.0625f };

        enemyConfigs[2].transformConfig.position    = { 9.0f, 0.0f };
        enemyConfigs[2].materialConfig.uvScale      = { 0.0625f, 0.0625f };
        enemyConfigs[2].materialConfig.uvOffset     = { 0.0f, 0.0625f };

        enemyConfigs[3].transformConfig.position    = { 12.0f, 0.0f };
        enemyConfigs[3].materialConfig.uvScale      = { 0.0625f, 0.0625f };
        enemyConfigs[3].materialConfig.uvOffset     = { 0.0f, 0.0625f };

        levels->enemies     = enemyConfigs;
        levels->enemyCount  = enemyCount;

        const auto obstacleCount    = *allocator->requestMemory<uint64_t>(1);
        auto* obstacleConfigs       = allocator->requestMemoryArray<ZeroXBridge::ObstacleConfig>(obstacleCount);

        obstacleConfigs[0].transformConfig.position = { 0.0f, -8.0f };
        obstacleConfigs[0].transformConfig.scale    = { 32.0f, 1.0f };
        obstacleConfigs[0].colliderConfig.size      = { 32.0f, 1.0f };
        obstacleConfigs[0].materialConfig.uvScale   = { 0.0625f, 0.0625f };
        obstacleConfigs[0].materialConfig.uvOffset  = { 0.0625f, 0.0625f };
        obstacleConfigs[0].materialConfig.tileSizeX = 16;

        levels->obstacles       = obstacleConfigs;
        levels->obstacleCount   = obstacleCount;

        context.levels      = levels;
        context.levelCount  = levelCount;

        return context;
    }

    bool loadLevel(const ZeroXBridge::GameContext* context, GameAllocator* allocator, size_t index)
    {
        if (!context)
        {
            return false;
        }

        if (index >= context->levelCount)
        {
            return false;
        }

        const GameMemory gameMemory = readGameMemory(allocator);

        allocator->free(allocator->getLiveSizeInBytes() - getReservedSize(gameMemory));
        const size_t baseLevelOffset = allocator->getLiveSizeInBytes();

        const ZeroXBridge::Level* level = context->levels + index;

        const auto backgroundCount  = *allocator->requestMemory<uint64_t>(level->backgroundCount);
        auto* backgrounds           = allocator->requestMemoryArray<Background>(backgroundCount);
        for (size_t i = 0; i < backgroundCount; ++i)
        {
            Background* background                      = backgrounds + i;
            const ZeroXBridge::BackgroundConfig* config = level->backgrounds + i;

            setPosition(background->entity.renderEntity.transform, config->transformConfig.position);
            setLocalScale(background->entity.renderEntity.transform, config->transformConfig.scale);

            background->entity.renderEntity.material    = config->materialConfig;
            background->entity.renderEntity.layer       = static_cast<uint64_t>(i);

            const float multiplier  = (i % 2) == 0 ? -1.0f : 1.0f;
            background->scrollSpeed = { ( 1.0f / static_cast<float>(i + 1) ) * 0.0625f * multiplier, 0.0f};
        }

        const auto playerCount  = *allocator->requestMemory<uint64_t>(level->playerCount);
        auto* players           = allocator->requestMemoryArray<Player>(playerCount);
        for (size_t i = 0; i < playerCount; ++i)
        {
            Player* player                          = players + i;
            const ZeroXBridge::PlayerConfig* config = level->players + i;

            setPosition(player->gameEntity.renderEntity.transform, config->transformConfig.position);
            setLocalScale(player->gameEntity.renderEntity.transform, config->transformConfig.scale);

            player->gameEntity.physicsState.currentPosition = config->transformConfig.position;
            player->gameEntity.physicsState.lastPosition    = config->transformConfig.position;

            player->gameEntity.renderEntity.material = config->materialConfig;
        }

        const auto enemyCount   = *allocator->requestMemory<uint64_t>(level->enemyCount);
        auto* enemies           = allocator->requestMemoryArray<Enemy>(enemyCount);
        for (size_t i = 0; i < enemyCount; ++i)
        {
            Enemy* enemy                            = enemies + i;
            const ZeroXBridge::EnemyConfig* config  = level->enemies + i;

            setPosition(enemy->gameEntity.renderEntity.transform, config->transformConfig.position);
            setLocalScale(enemy->gameEntity.renderEntity.transform, config->transformConfig.scale);

            enemy->gameEntity.physicsState.currentPosition  = config->transformConfig.position;
            enemy->gameEntity.physicsState.lastPosition     = config->transformConfig.position;

            enemy->gameEntity.renderEntity.material = config->materialConfig;
        }

        const auto obstacleCount    = *allocator->requestMemory<uint64_t>(level->obstacleCount);
        auto* obstacles             = allocator->requestMemoryArray<Obstacle>(obstacleCount);
        for (size_t i = 0; i < obstacleCount; ++i)
        {
            Obstacle* obstacle                          = obstacles + i;
            const ZeroXBridge::ObstacleConfig* config   = level->obstacles + i;

            setPosition(obstacle->gameEntity.renderEntity.transform, config->transformConfig.position);
            setLocalScale(obstacle->gameEntity.renderEntity.transform, config->transformConfig.scale);

            obstacle->gameEntity.physicsState.currentPosition   = config->transformConfig.position;
            obstacle->gameEntity.physicsState.lastPosition      = config->transformConfig.position;

            obstacle->gameEntity.renderEntity.material = config->materialConfig;
        }

        const auto colliderCount    = *allocator->requestMemory<uint64_t>(playerCount + enemyCount + obstacleCount);
        auto* colliders             = allocator->requestMemoryArray<Collider>(colliderCount);
        for (size_t i = 0; i < colliderCount; ++i)
        {
            Collider* collider = colliders + i;

            if (i < playerCount)
            {
                collider->target    = &(players + i)->gameEntity.renderEntity.transform;
                collider->size      = (level->players + i)->colliderConfig.size;
            }
            else if (i - playerCount < enemyCount)
            {
                collider->target    = &(enemies + i - playerCount)->gameEntity.renderEntity.transform;
                collider->size      = (level->enemies + i - playerCount)->colliderConfig.size;
            }
            else if (i - playerCount - enemyCount < obstacleCount)
            {
                collider->target    = &(obstacles + i - playerCount - enemyCount)->gameEntity.renderEntity.transform;
                collider->size      = (level->obstacles + i - playerCount - enemyCount)->colliderConfig.size;
            }
            else
            {
                collider->size      = { 0.0f, 0.0f };
                collider->target    = nullptr;
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

        const size_t tiledSpriteCount   = obstacleCount;
        auto* tiledSpriteEntitySection  = allocator->requestMemory<EntitySection>(
            EntitySection{
                .type   = EntitySectionType::TILED_SPRITE,
                .flags  = EntitySectionFlags::ALLOCATE_BATCH
            }
        );

        ++(*entitySectionCount);

        auto* colliderEntitySection = allocator->requestMemory<EntitySection>(EntitySection{
            .type   = EntitySectionType::COLLIDER,
            .flags  = EntitySectionFlags::ALLOCATE_BATCH });

        ++(*entitySectionCount);

        auto* backgroundReadDescriptors     = allocator->requestMemoryArray<EntityReadDescriptor>(backgroundCount);
        auto* spriteReadDescriptors         = allocator->requestMemoryArray<EntityReadDescriptor>(spriteCount);
        auto* tiledSpriteReadDescriptors    = allocator->requestMemoryArray<EntityReadDescriptor>(tiledSpriteCount);
        auto* colliderReadDescriptors       = allocator->requestMemoryArray<EntityReadDescriptor>(colliderCount);

        // Set entity section offsets

        MemoryCursor<16> entityCursor(baseLevelOffset);

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

            entityCursor.step<Player>();
        }

        entityCursor.step<uint64_t>();
        for (size_t i = 0; i < enemyCount; ++i)
        {
            *(spriteReadDescriptors + playerCount + i) = { .start = entityCursor.getOffset() };

            entityCursor.step<Enemy>();
        }

        spriteEntitySection->readDescriptors = spriteReadDescriptors;
        spriteEntitySection->descriptorCount = spriteCount;

        entityCursor.step<uint64_t>();
        for (size_t i = 0; i < obstacleCount; ++i)
        {
            *(tiledSpriteReadDescriptors + i) = { .start = entityCursor.getOffset() };

            entityCursor.step<Obstacle>();
        }

        tiledSpriteEntitySection->readDescriptors = tiledSpriteReadDescriptors;
        tiledSpriteEntitySection->descriptorCount = tiledSpriteCount;

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
        double physicsAccumulator{ 0.0 };

        step(
            readGameMemory(allocator),
            deltaTime,
            1.0 / 90,
            &physicsAccumulator);
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