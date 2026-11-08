#include "zerox_game.h"
#include "zerox_game_logic.h"

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
        auto* levels            = allocator->requestMemoryArray<Level>(1);

        const auto backgroundCount  = *allocator->requestMemory<uint64_t>(2);
        auto* backgrounds           = allocator->requestMemoryArray<Background>(backgroundCount);

        setPosition(backgrounds[0].entity.renderEntity.transform, { 0.0f, 0.0f });
        setLocalScale(backgrounds[0].entity.renderEntity.transform, { 33.0f, 16.875f });

        setPosition(backgrounds[1].entity.renderEntity.transform, { 0.0f, 0.0f });
        setLocalScale(backgrounds[1].entity.renderEntity.transform, { 33.0f, 16.875f });

        levels->backgrounds     = backgrounds;
        levels->backgroundCount = backgroundCount;

        const auto playerCount  = *allocator->requestMemory<uint64_t>(1);
        auto* players           = allocator->requestMemoryArray<Player>(playerCount);

        setPosition(players[0].gameEntity.renderEntity.transform, { 0.0f, 0.0f });
        players[0].gameEntity.renderEntity.material.uvScale = { 0.0625f, 0.0625f };

        levels->players     = players;
        levels->playerCount = playerCount;

        const auto enemyCount   = *allocator->requestMemory<uint64_t>(4);
        auto* enemies           = allocator->requestMemoryArray<Enemy>(enemyCount);

        setPosition(enemies[0].gameEntity.renderEntity.transform, { 3.0f, 0.0f });
        enemies[0].gameEntity.renderEntity.material.uvScale     = { 0.0625f, 0.0625f };
        enemies[0].gameEntity.renderEntity.material.uvOffset    = { 0.0f, 0.1875f };

        setPosition(enemies[1].gameEntity.renderEntity.transform, { 6.0f, 0.0f });
        enemies[1].gameEntity.renderEntity.material.uvScale     = { 0.0625f, 0.0625f };
        enemies[1].gameEntity.renderEntity.material.uvOffset    = { 0.0f, 0.1875f };

        setPosition(enemies[2].gameEntity.renderEntity.transform, { 9.0f, 0.0f });
        enemies[2].gameEntity.renderEntity.material.uvScale     = { 0.0625f, 0.0625f };
        enemies[2].gameEntity.renderEntity.material.uvOffset    = { 0.0f, 0.1875f };

        setPosition(enemies[3].gameEntity.renderEntity.transform, { 12.0f, 0.0f });
        enemies[3].gameEntity.renderEntity.material.uvScale     = { 0.0625f, 0.0625f };
        enemies[3].gameEntity.renderEntity.material.uvOffset    = { 0.0f, 0.1875f };

        levels->enemies     = enemies;
        levels->enemyCount  = enemyCount;

        const auto obstacleCount    = *allocator->requestMemory<uint64_t>(4);
        auto* obstacles             = allocator->requestMemoryArray<Obstacle>(obstacleCount);

        setPosition(obstacles[0].gameEntity.renderEntity.transform, { -15.25f, 0.0f });
        setLocalScale(obstacles[0].gameEntity.renderEntity.transform, { 1.0f, 18.0f });

        setPosition(obstacles[1].gameEntity.renderEntity.transform, { 15.25f, 0.0f });
        setLocalScale(obstacles[1].gameEntity.renderEntity.transform, { 1.0f, 18.0f });

        setPosition(obstacles[2].gameEntity.renderEntity.transform, { 0.0f, -8.0f });
        setLocalScale(obstacles[2].gameEntity.renderEntity.transform, { 32.0f, 1.0f });

        setPosition(obstacles[3].gameEntity.renderEntity.transform, { 0.0f, 8.0f });
        setLocalScale(obstacles[3].gameEntity.renderEntity.transform, { 32.0f, 1.0f });

        obstacles[0].gameEntity.renderEntity.material.uvScale   = { 0.0625f, 0.0625f };
        obstacles[0].gameEntity.renderEntity.material.uvOffset  = { 0.125f, 0.9375f };
        obstacles[0].gameEntity.renderEntity.material.tileSizeY = 16;

        obstacles[1].gameEntity.renderEntity.material.uvScale   = { 0.0625f, 0.0625f };
        obstacles[1].gameEntity.renderEntity.material.uvOffset  = { 0.125f, 0.9375f };
        obstacles[1].gameEntity.renderEntity.material.tileSizeY = 16;

        obstacles[2].gameEntity.renderEntity.material.uvScale   = { 0.0625f, 0.0625f };
        obstacles[2].gameEntity.renderEntity.material.uvOffset  = { 0.0f, 0.9375f };
        obstacles[2].gameEntity.renderEntity.material.tileSizeX = 16;

        obstacles[3].gameEntity.renderEntity.material.uvScale   = { 0.0625f, 0.0625f };
        obstacles[3].gameEntity.renderEntity.material.uvOffset  = { 0.0625f, 0.9375f };
        obstacles[3].gameEntity.renderEntity.material.tileSizeX = 32;

        levels->obstacles       = obstacles;
        levels->obstacleCount   = obstacleCount;

        return context;
    }

    bool loadLevel(const ZeroXBridge::GameContext* context, GameAllocator* allocator, size_t index)
    {
        if (!context)
        {
            return false;
        }

        const GameMemory gameMemory = readGameMemory(allocator);

        allocator->free(allocator->getLiveSizeInBytes() - getReservedSize(gameMemory));
        const size_t baseLevelOffset = allocator->getLiveSizeInBytes();

        const Level* level = gameMemory.levels.data() + index;

        const auto backgroundCount  = *allocator->requestMemory<uint64_t>(level->backgroundCount);
        auto* backgrounds           = allocator->requestMemoryArray<Background>(backgroundCount);
        for (size_t i = 0; i < backgroundCount; ++i)
        {
            Background* background      = backgrounds + i;
            const Background* config    = level->backgrounds + i;

            background->entity                              = config->entity;
            background->entity.physicsState.currentPosition = getPosition(background->entity.renderEntity.transform);
            background->entity.physicsState.lastPosition    = background->entity.physicsState.currentPosition;

            background->entity.renderEntity.layer   = static_cast<uint64_t>(i);
            const float multiplier                  = (i % 2) == 0 ? -1.0f : 1.0f;
            background->scrollSpeed                 = { ( 1.0f / static_cast<float>(i + 1) ) * 0.0625f * multiplier, 0.0f};
        }

        const auto playerCount  = *allocator->requestMemory<uint64_t>(level->playerCount);
        auto* players           = allocator->requestMemoryArray<Player>(playerCount);
        for (size_t i = 0; i < playerCount; ++i)
        {
            Player* player          = players + i;
            const Player* config    = level->players + i;

            player->gameEntity                              = config->gameEntity;
            player->gameEntity.physicsState.currentPosition = getPosition(player->gameEntity.renderEntity.transform);
            player->gameEntity.physicsState.lastPosition    = player->gameEntity.physicsState.currentPosition;
        }

        const auto enemyCount   = *allocator->requestMemory<uint64_t>(level->enemyCount);
        auto* enemies           = allocator->requestMemoryArray<Enemy>(enemyCount);
        for (size_t i = 0; i < enemyCount; ++i)
        {
            Enemy* enemy        = enemies + i;
            const Enemy* config = level->enemies + i;

            enemy->gameEntity                              = config->gameEntity;
            enemy->gameEntity.physicsState.currentPosition = getPosition(enemy->gameEntity.renderEntity.transform);
            enemy->gameEntity.physicsState.lastPosition    = enemy->gameEntity.physicsState.currentPosition;
        }

        const auto obstacleCount    = *allocator->requestMemory<uint64_t>(level->obstacleCount);
        auto* obstacles             = allocator->requestMemoryArray<Obstacle>(obstacleCount);
        for (size_t i = 0; i < obstacleCount; ++i)
        {
            Obstacle* obstacle      = obstacles + i;
            const Obstacle* config  = level->obstacles + i;

            obstacle->gameEntity                              = config->gameEntity;
            obstacle->gameEntity.physicsState.currentPosition = getPosition(obstacle->gameEntity.renderEntity.transform);
            obstacle->gameEntity.physicsState.lastPosition    = obstacle->gameEntity.physicsState.currentPosition;
        }

        const auto colliderCount    = *allocator->requestMemory<uint64_t>(playerCount + enemyCount + obstacleCount);
        auto* colliders             = allocator->requestMemoryArray<Collider>(colliderCount);

        for (size_t i = 0; i < playerCount; ++i)
        {
            colliders->size     = getScale((players + i)->gameEntity.renderEntity.transform);
            colliders->target   = &(players + i)->gameEntity.renderEntity.transform;

            setLocalScale(colliders->gameEntity.renderEntity.transform, colliders->size);
            colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliders;
        }

        for (size_t i = 0; i < enemyCount; ++i)
        {
            colliders->size     = getScale((enemies + i)->gameEntity.renderEntity.transform);
            colliders->target   = &(enemies + i)->gameEntity.renderEntity.transform;

            setLocalScale(colliders->gameEntity.renderEntity.transform, colliders->size);
            colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliders;
        }

        int sign{ 1 };
        for (size_t i = 0; i < obstacleCount; ++i)
        {
            colliders->size     = getScale((obstacles + i)->gameEntity.renderEntity.transform);
            colliders->target   = &(obstacles + i)->gameEntity.renderEntity.transform;
            colliders->normal   = static_cast<float>(sign) * ( i < 2 ? glm::vec2{ 1.0f, 0.0f } : glm::vec2{ 0.0f, 1.0f } ); // TODO(Karim): Base on obstacle type?

            setLocalScale(colliders->gameEntity.renderEntity.transform, colliders->size);
            colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliders;

            sign = -sign;
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