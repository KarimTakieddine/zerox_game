#include <zerox_audio.h>
#include <zerox_texture.h>

#include "zerox_enemy_types.h"
#include "zerox_game.h"
#include "zerox_game_animation.h"
#include "zerox_game_globals.h"
#include "zerox_game_logic.h"
#include "zerox_obstacle_types.h"

namespace
{
    struct PixelRegion
    {
        unsigned int x      { 0 };
        unsigned int y      { 0 };
        unsigned int width  { 0 };
        unsigned int height { 0 };
    };

    using ZeroXGame::Animation;
    using ZeroXGame::AnimationFlags;
    using ZeroXGame::AnimationFrame;

    Animation* allocateAnimations(ZeroXGame::GameAllocator* allocator, size_t count)
    {
        allocator->requestMemory<uint64_t>(static_cast<uint64_t>(count));
        return allocator->requestMemoryArray<Animation>(count);
    }

    void allocateFrames(ZeroXGame::GameAllocator* allocator, Animation* animation, const PixelRegion& sampleRegion, const PixelRegion& animationRegion, unsigned int frameWidth, unsigned int frameHeight)
    {
        const unsigned int columnCount  { animationRegion.width / frameWidth };
        const unsigned int rowCount     { animationRegion.height / frameHeight };

        animation->frameCount   = *allocator->requestMemory<uint64_t>(static_cast<uint64_t>(rowCount * columnCount));
        animation->frames       = allocator->requestMemoryArray<AnimationFrame>(animation->frameCount);

        const glm::vec2 uvScale = {
            static_cast<float>(frameWidth) / sampleRegion.width,
            static_cast<float>(frameHeight) / sampleRegion.height
        };

        const glm::vec2 uvOffset = {
            static_cast<float>(animationRegion.x) / sampleRegion.width,
            static_cast<float>(animationRegion.y) / sampleRegion.height
        };

        for (unsigned int row = 0; row < rowCount; ++row)
        {
            for (unsigned int column = 0; column < columnCount; ++column)
            {
                const size_t flattenedIndex = static_cast<size_t>(row * columnCount) + column;

                *(animation->frames + flattenedIndex) = {
                    .uvScale    = uvScale,
                    .uvOffset   = uvOffset + glm::vec2{ column * uvScale.x, row * uvScale.y }
                };
            }
        }
    }
}

namespace ZeroXGame
{
    using ZeroXBridge::EntitySection;
    using ZeroXBridge::EntitySectionType;
    using ZeroXBridge::EntitySectionFlags;
    using ZeroXBridge::EntityReadDescriptor;
    using ZeroXBridge::RenderDescriptor;

    void setEngineFunctions(const ZeroXBridge::EngineFunctions* engineFunctions)
    {
        globalEngineFunctions = engineFunctions;
    }

    ZeroXBridge::GameContext initializeGame(GameAllocator* allocator, const ZeroXBridge::EngineFunctions* engineFunctions)
    {
        ZeroXBridge::GameContext context = { .input = allocator->requestMemory<ZeroXBridge::Input>() };

        if (!allocator || !engineFunctions)
        {
            return context;
        }

        setEngineFunctions(engineFunctions);

        if (!ZeroXBridge::isValid(globalEngineFunctions))
        {
            return context;
        }

        constexpr const char* textureFiles[1] = {
            "./textures/the_mega_texture.png"
        };

        ZeroXBridge::Texture theMegaTexture = {
            .width      = 1024,
            .height     = 1024,
            .format     = ZeroXBridge::TextureFormat::RGBA,
            .wrapModeS  = ZeroXBridge::TextureWrapMode::CLAMP_TO_EDGE,
            .wrapModeT  = ZeroXBridge::TextureWrapMode::CLAMP_TO_EDGE,
            .minFilter  = ZeroXBridge::TextureFilter::NEAREST,
            .magFilter  = ZeroXBridge::TextureFilter::NEAREST
        };

        engineFunctions->uploadTextures(engineFunctions->handle, textureFiles, &theMegaTexture, 1);

        constexpr const char* textureArrayFiles[2] = {
            "./textures/test_background_0.png",
            "./textures/test_background_1.png"
        };

        ZeroXBridge::Texture backgroundTexture = {
            .width		= 1920,
			.height		= 1080,
            .format     = ZeroXBridge::TextureFormat::RGBA,
			.wrapModeS	= ZeroXBridge::TextureWrapMode::REPEAT,
			.wrapModeT	= ZeroXBridge::TextureWrapMode::REPEAT,
			.minFilter	= ZeroXBridge::TextureFilter::LINEAR,
			.magFilter	= ZeroXBridge::TextureFilter::LINEAR
        };

        engineFunctions->uploadTextureArray(engineFunctions->handle, textureArrayFiles, 2, &backgroundTexture, 1);

        ZeroXBridge::AudioSound characterJumpSound{
            .type   = ZeroXBridge::SoundType::SOUND_EFFECT,
            .flags  = ZeroXBridge::SoundFlags::NONE
        };
        engineFunctions->allocateAudio(engineFunctions->handle, "./audio/character_jump.mp3", &characterJumpSound);

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

        const auto skullCount   = *allocator->requestMemory<uint64_t>(4);
        auto* skulls            = allocator->requestMemoryArray<Skull>(skullCount);

        setPosition(skulls[0].gameEntity.renderEntity.transform, { 3.0f, 0.0f });
        skulls[0].gameEntity.renderEntity.material.uvScale     = { 0.0625f, 0.0625f };
        skulls[0].gameEntity.renderEntity.material.uvOffset    = { 0.0f, 0.1875f };

        setPosition(skulls[1].gameEntity.renderEntity.transform, { 6.0f, 0.0f });
        skulls[1].gameEntity.renderEntity.material.uvScale     = { 0.0625f, 0.0625f };
        skulls[1].gameEntity.renderEntity.material.uvOffset    = { 0.0f, 0.1875f };

        setPosition(skulls[2].gameEntity.renderEntity.transform, { 9.0f, 0.0f });
        skulls[2].gameEntity.renderEntity.material.uvScale     = { 0.0625f, 0.0625f };
        skulls[2].gameEntity.renderEntity.material.uvOffset    = { 0.0f, 0.1875f };

        setPosition(skulls[3].gameEntity.renderEntity.transform, { 12.0f, 0.0f });
        skulls[3].gameEntity.renderEntity.material.uvScale     = { 0.0625f, 0.0625f };
        skulls[3].gameEntity.renderEntity.material.uvOffset    = { 0.0f, 0.1875f };

        levels->skulls      = skulls;
        levels->skullCount  = skullCount;

        const auto mounsCount   = *allocator->requestMemory<uint64_t>(1);
        auto* mounsList         = allocator->requestMemoryArray<Mouns>(mounsCount);

        setPosition(mounsList[0].gameEntity.renderEntity.transform, { 7.0f, -4.0f });
        mounsList[0].gameEntity.renderEntity.material.uvScale = { 0.0625f, 0.0625f };
        mounsList[0].gameEntity.renderEntity.material.uvOffset = { 0.0f, 0.125f };

        levels->mounsList   = mounsList;
        levels->mounsCount  = mounsCount;

        const auto obstacleCount    = *allocator->requestMemory<uint64_t>(4);
        auto* obstacles             = allocator->requestMemoryArray<Obstacle>(obstacleCount);

        setPosition(obstacles[0].gameEntity.renderEntity.transform, { -15.25f, 0.0f });
        setLocalScale(obstacles[0].gameEntity.renderEntity.transform, { 1.0f, 18.0f });
        obstacles[0].collider = { .size = { 1.0f, 18.0f }, .normal = { 1.0f, 0.0f } };

        setPosition(obstacles[1].gameEntity.renderEntity.transform, { 15.25f, 0.0f });
        setLocalScale(obstacles[1].gameEntity.renderEntity.transform, { 1.0f, 18.0f });
        obstacles[1].collider = { .size = { 1.0f, 18.0f }, .normal = { -1.0f, 0.0f } };

        setPosition(obstacles[2].gameEntity.renderEntity.transform, { 0.0f, -8.0f });
        setLocalScale(obstacles[2].gameEntity.renderEntity.transform, { 32.0f, 1.0f });
        obstacles[2].collider = { .size = { 32.0f, 1.0f }, .normal = { 0.0f, 1.0f } };

        setPosition(obstacles[3].gameEntity.renderEntity.transform, { 0.0f, 8.0f });
        setLocalScale(obstacles[3].gameEntity.renderEntity.transform, { 32.0f, 1.0f });
        obstacles[3].collider = { .size = { 32.0f, 1.0f }, .normal = { 0.0f, -1.0f } };

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
        obstacles[3].gameEntity.renderEntity.material.uvOffset  = { 0.0f, 0.9375f };
        obstacles[3].gameEntity.renderEntity.material.tileSizeX = 16;

        levels->obstacles       = obstacles;
        levels->obstacleCount   = obstacleCount;

        const auto movingPlatformCount  = *allocator->requestMemory<uint64_t>(1);
        auto* movingPlatforms           = allocator->requestMemoryArray<MovingPlatform>(movingPlatformCount);

        setPosition(movingPlatforms[0].obstacle.gameEntity.renderEntity.transform, { -6.0f, 2.0f });
        setLocalScale(movingPlatforms[0].obstacle.gameEntity.renderEntity.transform, { 4.0f, 1.0f });
        movingPlatforms[0].obstacle.collider.size   = { 4.0f, 1.0f };
        movingPlatforms[0].obstacle.collider.normal = { 0.0f, 1.0f };
        movingPlatforms[0].obstacle.gameEntity.renderEntity.material.uvScale    = { 0.0625f, 0.0625f };
        movingPlatforms[0].obstacle.gameEntity.renderEntity.material.uvOffset   = { 0.0f, 0.9375f };
        movingPlatforms[0].obstacle.gameEntity.renderEntity.material.tileSizeX  = 2;
        movingPlatforms[0].switchTime                                           = 8.0f;
        movingPlatforms[0].velocity                                             = { -0.0f, -1.5f };

        levels->movingPlatforms     = movingPlatforms;
        levels->movingPlatformCount = movingPlatformCount;

        auto* animations = ::allocateAnimations(allocator, 3);

        auto* jumpAnimation = animations;
        ::allocateFrames(
            allocator,
            jumpAnimation,
            { .x = 0, .y = 0, .width = 1024, .height = 1024 },
            { .x = 0, .y = 64, .width = 512, .height = 64 },
            64,
            64);

        auto* fallAnimation = animations + 1;
        ::allocateFrames(
            allocator,
            fallAnimation,
            { .x = 0, .y = 0, .width = 1024, .height = 1024 },
            { .x = 832, .y = 0, .width = 192, .height = 64 },
            64,
            64);

        auto* walkAnimation = animations + 2;
        ::allocateFrames(
            allocator,
            walkAnimation,
            { .x = 0, .y = 0, .width = 1024, .height = 1024 },
            { .x = 320, .y = 0, .width = 320, .height = 64 },
            64,
            64);

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
            player->collider                                = config->collider;
            player->gameEntity.physicsState.currentPosition = getPosition(player->gameEntity.renderEntity.transform);
            player->gameEntity.physicsState.lastPosition    = player->gameEntity.physicsState.currentPosition;
        }

        const auto skullCount   = *allocator->requestMemory<uint64_t>(level->skullCount);
        auto* skulls            = allocator->requestMemoryArray<Skull>(skullCount);
        for (size_t i = 0; i < skullCount; ++i)
        {
            Skull* skull        = skulls + i;
            const Skull* config = level->skulls + i;

            skull->gameEntity                               = config->gameEntity;
            skull->collider                                 = config->collider;
            skull->gameEntity.physicsState.currentPosition  = getPosition(skull->gameEntity.renderEntity.transform);
            skull->gameEntity.physicsState.lastPosition     = skull->gameEntity.physicsState.currentPosition;
        }

        const auto mounsCount   = *allocator->requestMemory<uint64_t>(level->mounsCount);
        auto* mounsList         = allocator->requestMemoryArray<Mouns>(mounsCount);
        for (size_t i = 0; i < mounsCount; ++i)
        {
            Mouns* mouns        = mounsList + i;
            const Mouns* config = level->mounsList + i;

            mouns->gameEntity                               = config->gameEntity;
            mouns->collider                                 = config->collider;
            mouns->gameEntity.physicsState.currentPosition  = getPosition(mouns->gameEntity.renderEntity.transform);
            mouns->gameEntity.physicsState.lastPosition     = mouns->gameEntity.physicsState.currentPosition;
        }

        const auto obstacleCount    = *allocator->requestMemory<uint64_t>(level->obstacleCount);
        auto* obstacles             = allocator->requestMemoryArray<Obstacle>(obstacleCount);
        for (size_t i = 0; i < obstacleCount; ++i)
        {
            Obstacle* obstacle      = obstacles + i;
            const Obstacle* config  = level->obstacles + i;

            obstacle->gameEntity                                = config->gameEntity;
            obstacle->collider                                  = config->collider;
            obstacle->gameEntity.physicsState.currentPosition   = getPosition(obstacle->gameEntity.renderEntity.transform);
            obstacle->gameEntity.physicsState.lastPosition      = obstacle->gameEntity.physicsState.currentPosition;
        }

        const auto movingPlatformCount  = *allocator->requestMemory<uint64_t>(1);
        auto* movingPlatforms           = allocator->requestMemoryArray<MovingPlatform>(movingPlatformCount);
        for (size_t i = 0; i < movingPlatformCount; ++i)
        {
            MovingPlatform* movingPlatform  = movingPlatforms + i;
            const MovingPlatform* config    = level->movingPlatforms + i;

            movingPlatform->obstacle.gameEntity                                 = config->obstacle.gameEntity;
            movingPlatform->obstacle.collider                                   = config->obstacle.collider;
            movingPlatform->obstacle.gameEntity.physicsState.currentPosition    = getPosition(movingPlatform->obstacle.gameEntity.renderEntity.transform);
            movingPlatform->obstacle.gameEntity.physicsState.lastPosition       = movingPlatform->obstacle.gameEntity.physicsState.currentPosition;
            movingPlatform->switchTime                                          = config->switchTime;
            movingPlatform->velocity                                            = config->velocity;
        }

        const auto colliderCount    = *allocator->requestMemory<uint64_t>(playerCount + skullCount + mounsCount + obstacleCount + movingPlatformCount);
        auto* colliders             = allocator->requestMemoryArray<Collider2DEntity>(colliderCount);

        for (size_t i = 0; i < playerCount; ++i)
        {
            colliders->collider2D   = &(players + i)->collider;
            colliders->target       = &(players + i)->gameEntity.renderEntity.transform;

            setLocalScale(colliders->gameEntity.renderEntity.transform, (players + i)->collider.size);
            colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliders;
        }

        for (size_t i = 0; i < skullCount; ++i)
        {
            colliders->collider2D   = &(skulls + i)->collider;
            colliders->target       = &(skulls + i)->gameEntity.renderEntity.transform;

            setLocalScale(colliders->gameEntity.renderEntity.transform, (skulls + i)->collider.size);
            colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliders;
        }

        for (size_t i = 0; i < mounsCount; ++i)
        {
            colliders->collider2D   = &(mounsList + i)->collider;
            colliders->target       = &(mounsList + i)->gameEntity.renderEntity.transform;

            setLocalScale(colliders->gameEntity.renderEntity.transform, (mounsList + i)->collider.size);
            colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliders;
        }

        for (size_t i = 0; i < obstacleCount; ++i)
        {
            colliders->collider2D   = &(obstacles + i)->collider;
            colliders->target       = &(obstacles + i)->gameEntity.renderEntity.transform;

            setLocalScale(colliders->gameEntity.renderEntity.transform, (obstacles + i)->collider.size);
            colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliders;
        }

        for (size_t i = 0; i < movingPlatformCount; ++i)
        {
            colliders->collider2D   = &(movingPlatforms + i)->obstacle.collider;
            colliders->target       = &(movingPlatforms + i)->obstacle.gameEntity.renderEntity.transform;

            setLocalScale(colliders->gameEntity.renderEntity.transform, (movingPlatforms + i)->obstacle.collider.size);
            colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliders;
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

        const size_t spriteCount    = playerCount + skullCount + mounsCount;
        auto* spriteEntitySection   = allocator->requestMemory<EntitySection>(EntitySection{
            .type   = EntitySectionType::SPRITE,
            .flags  = EntitySectionFlags::ALLOCATE_BATCH });

        ++(*entitySectionCount);

        const size_t tiledSpriteCount   = obstacleCount + movingPlatformCount;
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
        for (size_t i = 0; i < skullCount; ++i)
        {
            *(spriteReadDescriptors + playerCount + i) = { .start = entityCursor.getOffset() };

            entityCursor.step<Skull>();
        }

        entityCursor.step<uint64_t>();
        for (size_t i = 0; i < mounsCount; ++i)
        {
            *(spriteReadDescriptors + playerCount + skullCount + i) = { .start = entityCursor.getOffset() };

            entityCursor.step<Mouns>();
        }

        spriteEntitySection->readDescriptors = spriteReadDescriptors;
        spriteEntitySection->descriptorCount = spriteCount;

        entityCursor.step<uint64_t>();
        for (size_t i = 0; i < obstacleCount; ++i)
        {
            *(tiledSpriteReadDescriptors + i) = { .start = entityCursor.getOffset() };

            entityCursor.step<Obstacle>();
        }

        entityCursor.step<uint64_t>();
        for (size_t i = 0; i < movingPlatformCount; ++i)
        {
            *(tiledSpriteReadDescriptors + obstacleCount + i) = { .start = entityCursor.getOffset() };

            entityCursor.step<MovingPlatform>();
        }

        tiledSpriteEntitySection->readDescriptors = tiledSpriteReadDescriptors;
        tiledSpriteEntitySection->descriptorCount = tiledSpriteCount;

        entityCursor.step<uint64_t>();
        for (size_t i = 0; i < colliderCount; ++i)
        {
            *(colliderReadDescriptors + i) = { .start = entityCursor.getOffset() };

            entityCursor.step<Collider2DEntity>();
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