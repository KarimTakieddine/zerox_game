#include <zerox_audio.h>
#include <zerox_texture.h>

#include "zerox_game.h"
#include "zerox_game_globals.h"
#include "zerox_game_logic.h"
#include "zerox_game_transform.h"

#include "zerox_game_memory_header.h"
#include "zerox_game_memory_reader.hpp"

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

        const size_t frameCount = static_cast<size_t>(rowCount) * columnCount;

        auto* frames = allocator->requestMemoryArray<AnimationFrame>(frameCount);

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

                *(frames + flattenedIndex) = {
                    .uvScale    = uvScale,
                    .uvOffset   = uvOffset + glm::vec2{ column * uvScale.x, row * uvScale.y }
                };
            }
        }

        animation->frames       = frames;
        animation->frameCount   = frameCount;
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
        if (!allocator || !engineFunctions)
        {
            return { };
        }

        setEngineFunctions(engineFunctions);

        if (!ZeroXBridge::isValid(globalEngineFunctions))
        {
            return { };
        }

        constexpr const char* textureFiles[1] = {
            "../../../textures/the_mega_texture.png"
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
            "../../../textures/test_background_0.png",
            "../../../textures/test_background_1.png"
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
        engineFunctions->allocateAudio(engineFunctions->handle, "../../../audio/character_jump.mp3", &characterJumpSound);

        allocator->requestMemory<GameMemoryHeader>(GameMemoryHeader{ .type = MemoryType::INPUT, .count = 1 });
        auto* input = allocator->requestMemoryArray<ZeroXBridge::Input>(1);

        allocator->requestMemory<GameMemoryHeader>(GameMemoryHeader{ .type = MemoryType::LEVEL, .count = 1 });
        auto* levels = allocator->requestMemoryArray<LevelView>(1);

        const size_t animationCount{ 3 };
        allocator->requestMemory<GameMemoryHeader>(GameMemoryHeader{ .type = MemoryType::ANIMATION, .count = static_cast<uint64_t>(animationCount) });
        auto* animations = allocator->requestMemoryArray<Animation>(animationCount);

        // Build level(s):

        auto configureTransform = [](ZeroXBridge::Transform& transform, const glm::vec2& position = { 0.0f, 0.0f }, const glm::vec2& localScale = { 1.0f, 1.0f }) -> void {
            setPosition(transform, position);
            setLocalScale(transform, localScale);
        };

        auto configureCollider = [](Collider2D& collider, const glm::vec2& size = { 1.0f, 1.0f }, const glm::vec2& normal = { 0.0f, 0.0f }) -> void {
            collider.size   = size;
            collider.normal = normal;
        };

        auto configureMaterial = [](ZeroXBridge::Material& material, const glm::vec2& uvScale = { 1.0f, 1.0f }, const glm::vec2& uvOffset = { 0.0f, 0.0f }, uint64_t tileSizeX = 1, uint64_t tileSizeY = 1) -> void {
            material.uvScale    = uvScale;
            material.uvOffset   = uvOffset;
            material.tileSizeX  = tileSizeX;
            material.tileSizeY  = tileSizeY;
        };

            // -> Configure background(s):

        constexpr size_t backgroundCount{ 2 };
        auto* backgrounds = allocator->requestMemoryArray<Background>(backgroundCount);

        configureTransform(backgrounds[0].entity.renderEntity.transform, { 0.0f, 0.0f }, { 33.0f, 16.875f });
        configureTransform(backgrounds[1].entity.renderEntity.transform, { 0.0f, 0.0f }, { 33.0f, 16.875f });

            // -> Configure obstacle(s):

        constexpr size_t obstacleCount{ 4 };
        auto* obstacles = allocator->requestMemoryArray<Obstacle>(obstacleCount);

        configureTransform(obstacles[0].gameEntity.renderEntity.transform, { -15.25f, 0.0f }, { 1.0f, 18.0f });
        configureCollider(obstacles[0].collider, { 1.0f, 18.0f }, { 1.0f, 0.0f });

        configureTransform(obstacles[1].gameEntity.renderEntity.transform, { 15.25f, 0.0f }, { 1.0f, 18.0f });
        configureCollider(obstacles[1].collider, { 1.0f, 18.0f }, { -1.0f, 0.0f });

        configureTransform(obstacles[2].gameEntity.renderEntity.transform, { 0.0f, -8.0f }, { 32.0f, 1.0f });
        configureCollider(obstacles[2].collider, { 32.0f, 1.0f }, { 0.0f, 1.0f });

        configureTransform(obstacles[3].gameEntity.renderEntity.transform, { 0.0f, 8.0f }, { 32.0f, 1.0f });
        configureCollider(obstacles[3].collider, { 32.0f, 1.0f }, { 0.0f, -1.0f });

            // -> Configure moving platforms(s):

        constexpr size_t movingPlatformCount{ 1 };
        auto* movingPlatforms = allocator->requestMemoryArray<MovingPlatform>(movingPlatformCount);

        configureTransform(movingPlatforms[0].obstacle.gameEntity.renderEntity.transform, { -6.0f, 2.0f }, { 4.0f, 1.0f });
        configureCollider(movingPlatforms[0].obstacle.collider, { 4.0f, 1.0f }, { 0.0f, 1.0f });
        configureMaterial(movingPlatforms[0].obstacle.gameEntity.renderEntity.material, { 0.0625f, 0.0625f }, { 0.0f, 0.9375f }, 2);

        movingPlatforms[0].switchTime   = 8.0f;
        movingPlatforms[0].velocity     = { -0.0f, -1.5f };

            // -> Configure skull(s):
        
        constexpr size_t skullCount{ 4 };
        auto* skulls = allocator->requestMemoryArray<Skull>(skullCount);

        setPosition(skulls[0].gameEntity.renderEntity.transform, { 3.0f, 0.0f });
        configureMaterial(skulls[0].gameEntity.renderEntity.material, { 0.0625f, 0.0625f }, { 0.0f, 0.1875f });

        setPosition(skulls[1].gameEntity.renderEntity.transform, { 6.0f, 0.0f });
        configureMaterial(skulls[1].gameEntity.renderEntity.material, { 0.0625f, 0.0625f }, { 0.0f, 0.1875f });

        setPosition(skulls[2].gameEntity.renderEntity.transform, { 9.0f, 0.0f });
        configureMaterial(skulls[2].gameEntity.renderEntity.material, { 0.0625f, 0.0625f }, { 0.0f, 0.1875f });

        setPosition(skulls[3].gameEntity.renderEntity.transform, { 12.0f, 0.0f });
        configureMaterial(skulls[1].gameEntity.renderEntity.material, { 0.0625f, 0.0625f }, { 0.0f, 0.1875f });

            // -> Configure Mouns(s):

        constexpr size_t mounsCount{ 1 };
        auto* mounsList = allocator->requestMemoryArray<Mouns>(mounsCount);

        setPosition(mounsList[0].gameEntity.renderEntity.transform, { 7.0f, -4.0f });
        configureMaterial(mounsList[0].gameEntity.renderEntity.material, { 0.0625f, 0.0625f }, { 0.0f, 0.125f });

            // -> Configure Player(s):

        constexpr size_t playerCount{ 1 };
        auto* players = allocator->requestMemoryArray<Player>(playerCount);

        setPosition(players[0].gameEntity.renderEntity.transform, { 0.0f, 0.0f });
        players[0].gameEntity.renderEntity.material.uvScale = { 0.0625f, 0.0625f };

        levels->backgrounds     = std::span<Background>(backgrounds, backgroundCount);
        levels->obstacles       = std::span<Obstacle>(obstacles, obstacleCount);
        levels->movingPlatforms = std::span<MovingPlatform>(movingPlatforms, movingPlatformCount);
        levels->skulls          = std::span<Skull>(skulls, skullCount);
        levels->mounsList       = std::span<Mouns>(mounsList, mounsCount);
        levels->players         = std::span<Player>(players, playerCount);

            // -> Configure Animations(s):

        ::allocateFrames(allocator, animations, { .x = 0, .y = 0, .width = 1024, .height = 1024 }, { .x = 0, .y = 64, .width = 512, .height = 64 }, 64, 64);
        ::allocateFrames(allocator, animations + 1, { .x = 0, .y = 0, .width = 1024, .height = 1024 }, { .x = 832, .y = 0, .width = 192, .height = 64 }, 64, 64);
        ::allocateFrames(allocator, animations + 2, { .x = 0, .y = 0, .width = 1024, .height = 1024 }, { .x = 320, .y = 0, .width = 320, .height = 64 }, 64, 64);

        return { .input = input };
    }

    bool loadLevel(const ZeroXBridge::GameContext* context, GameAllocator* allocator, size_t index)
    {
        const GameMemoryView gameMemoryView = readGameMemory(reinterpret_cast<unsigned char*>(allocator->peek()));

        allocator->free(allocator->getLiveSizeInBytes() - getReservedByteSize(gameMemoryView));

        const auto levels = gameMemoryView.levels;
        if (index >= levels.size())
        {
            return false;
        }

        const auto* level = gameMemoryView.levels.data() + index;

        allocator->requestMemory<GameMemoryHeader>(GameMemoryHeader{ .type = MemoryType::LEVEL, .count = 1 });
        auto* liveLevel = allocator->requestMemoryArray<LevelView>(1);

        auto resetPhysics = [](const ZeroXBridge::Transform& transform, PhysicsState& state) -> void {
            state.currentPosition = getPosition(transform);
            state.lastPosition    = state.currentPosition;
        };

        const auto levelBackgrounds     = level->backgrounds;
        const size_t backgroundCount    = levelBackgrounds.size();
        auto* liveBackgrounds           = allocator->requestMemoryArray<Background>(backgroundCount);

        for (size_t i = 0; i < backgroundCount; ++i)
        {
            Background* liveBackground  = liveBackgrounds + i;
            *(liveBackground)           = *(levelBackgrounds.data() + i);

            resetPhysics(liveBackground->entity.renderEntity.transform, liveBackground->entity.physicsState);
        }

        const auto levelObstacles   = level->obstacles;
        const size_t obstacleCount  = levelObstacles.size();
        auto* liveObstacles         = allocator->requestMemoryArray<Obstacle>(obstacleCount);

        for (size_t i = 0; i < obstacleCount; ++i)
        {
            Obstacle* liveObstacle  = liveObstacles + i;
            *(liveObstacle)         = *(levelObstacles.data() + i);

            resetPhysics(liveObstacle->gameEntity.renderEntity.transform, liveObstacle->gameEntity.physicsState);
        }

        const auto levelMovingPlatforms     = level->movingPlatforms;
        const size_t movingPlatformCount    = levelMovingPlatforms.size();
        auto* liveMovingPlatforms           = allocator->requestMemoryArray<MovingPlatform>(movingPlatformCount);

        for (size_t i = 0; i < movingPlatformCount; ++i)
        {
            MovingPlatform* liveMovingPlatform  = liveMovingPlatforms + i;
            *(liveMovingPlatform)               = *(levelMovingPlatforms.data() + i);

            resetPhysics(liveMovingPlatform->obstacle.gameEntity.renderEntity.transform, liveMovingPlatform->obstacle.gameEntity.physicsState);
        }

        const auto levelSkulls  = level->skulls;
        const size_t skullCount = levelSkulls.size();
        auto* liveSkulls        = allocator->requestMemoryArray<Skull>(skullCount);

        for (size_t i = 0; i < skullCount; ++i)
        {
            Skull* liveSkull    = liveSkulls + i;
            *(liveSkull)        = *(levelSkulls.data() + i);

            resetPhysics(liveSkull->gameEntity.renderEntity.transform, liveSkull->gameEntity.physicsState);
        }

        const auto levelMouns   = level->mounsList;
        const size_t mounsCount = levelMouns.size();
        auto* liveMouns         = allocator->requestMemoryArray<Mouns>(mounsCount);

        for (size_t i = 0; i < mounsCount; ++i)
        {
            Mouns* mouns    = liveMouns + i;
            *(mouns)        = *(levelMouns.data() + i);

            resetPhysics(mouns->gameEntity.renderEntity.transform, mouns->gameEntity.physicsState);
        }

        const auto levelPlayers     = level->players;
        const size_t playerCount    = levelPlayers.size();
        auto* livePlayers           = allocator->requestMemoryArray<Player>(playerCount);

        liveLevel->backgrounds      = { liveBackgrounds, backgroundCount };
        liveLevel->obstacles        = { liveObstacles, obstacleCount };
        liveLevel->movingPlatforms  = { liveMovingPlatforms, movingPlatformCount };
        liveLevel->skulls           = { liveSkulls, skullCount };
        liveLevel->mounsList        = { liveMouns, mounsCount };
        liveLevel->players          = { livePlayers, playerCount };

        //const auto colliderCount    = *allocator->requestMemory<uint64_t>(playerCount + skullCount + mounsCount + obstacleCount + movingPlatformCount);
        //auto* colliders             = allocator->requestMemoryArray<Collider2DEntity>(colliderCount);

        //for (size_t i = 0; i < playerCount; ++i)
        //{
        //    colliders->collider2D   = &(players + i)->collider;
        //    colliders->target       = &(players + i)->gameEntity.renderEntity.transform;

        //    setLocalScale(colliders->gameEntity.renderEntity.transform, (players + i)->collider.size);
        //    colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

        //    ++colliders;
        //}

        //for (size_t i = 0; i < skullCount; ++i)
        //{
        //    colliders->collider2D   = &(skulls + i)->collider;
        //    colliders->target       = &(skulls + i)->gameEntity.renderEntity.transform;

        //    setLocalScale(colliders->gameEntity.renderEntity.transform, (skulls + i)->collider.size);
        //    colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

        //    ++colliders;
        //}

        //for (size_t i = 0; i < mounsCount; ++i)
        //{
        //    colliders->collider2D   = &(mounsList + i)->collider;
        //    colliders->target       = &(mounsList + i)->gameEntity.renderEntity.transform;

        //    setLocalScale(colliders->gameEntity.renderEntity.transform, (mounsList + i)->collider.size);
        //    colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

        //    ++colliders;
        //}

        //for (size_t i = 0; i < obstacleCount; ++i)
        //{
        //    colliders->collider2D   = &(obstacles + i)->collider;
        //    colliders->target       = &(obstacles + i)->gameEntity.renderEntity.transform;

        //    setLocalScale(colliders->gameEntity.renderEntity.transform, (obstacles + i)->collider.size);
        //    colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

        //    ++colliders;
        //}

        //for (size_t i = 0; i < movingPlatformCount; ++i)
        //{
        //    colliders->collider2D   = &(movingPlatforms + i)->obstacle.collider;
        //    colliders->target       = &(movingPlatforms + i)->obstacle.gameEntity.renderEntity.transform;

        //    setLocalScale(colliders->gameEntity.renderEntity.transform, (movingPlatforms + i)->obstacle.collider.size);
        //    colliders->gameEntity.renderEntity.material.color = { 0.0f, 1.0f, 0.0f, 1.0f };

        //    ++colliders;
        //}

        //// Allocate entity sections

        //auto* entitySectionCount = allocator->requestMemory<uint64_t>(0);

        //auto* backgroundEntitySection = allocator->requestMemory<EntitySection>(
        //    EntitySection{
        //        .type   = EntitySectionType::BACKGROUND,
        //        .flags  = EntitySectionFlags::ALLOCATE_BATCH
        //    }
        //);

        //++(*entitySectionCount);

        //const size_t spriteCount    = playerCount + skullCount + mounsCount;
        //auto* spriteEntitySection   = allocator->requestMemory<EntitySection>(EntitySection{
        //    .type   = EntitySectionType::SPRITE,
        //    .flags  = EntitySectionFlags::ALLOCATE_BATCH });

        //++(*entitySectionCount);

        //const size_t tiledSpriteCount   = obstacleCount + movingPlatformCount;
        //auto* tiledSpriteEntitySection  = allocator->requestMemory<EntitySection>(
        //    EntitySection{
        //        .type   = EntitySectionType::TILED_SPRITE,
        //        .flags  = EntitySectionFlags::ALLOCATE_BATCH
        //    }
        //);

        //++(*entitySectionCount);

        //auto* colliderEntitySection = allocator->requestMemory<EntitySection>(EntitySection{
        //    .type   = EntitySectionType::COLLIDER,
        //    .flags  = EntitySectionFlags::ALLOCATE_BATCH });

        //++(*entitySectionCount);

        //auto* backgroundReadDescriptors     = allocator->requestMemoryArray<EntityReadDescriptor>(backgroundCount);
        //auto* spriteReadDescriptors         = allocator->requestMemoryArray<EntityReadDescriptor>(spriteCount);
        //auto* tiledSpriteReadDescriptors    = allocator->requestMemoryArray<EntityReadDescriptor>(tiledSpriteCount);
        //auto* colliderReadDescriptors       = allocator->requestMemoryArray<EntityReadDescriptor>(colliderCount);

        //// Set entity section offsets

        //MemoryCursor<16> entityCursor(baseLevelOffset);

        //entityCursor.step<uint64_t>();
        //for (size_t i = 0; i < backgroundCount; ++i)
        //{
        //    *(backgroundReadDescriptors + i) = { .start = entityCursor.getOffset() };

        //    entityCursor.step<Background>();
        //}

        //backgroundEntitySection->readDescriptors = backgroundReadDescriptors;
        //backgroundEntitySection->descriptorCount = backgroundCount;

        //entityCursor.step<uint64_t>();
        //for (size_t i = 0; i < playerCount; ++i)
        //{
        //    *(spriteReadDescriptors + i) = { .start = entityCursor.getOffset() };

        //    entityCursor.step<Player>();
        //}

        //entityCursor.step<uint64_t>();
        //for (size_t i = 0; i < skullCount; ++i)
        //{
        //    *(spriteReadDescriptors + playerCount + i) = { .start = entityCursor.getOffset() };

        //    entityCursor.step<Skull>();
        //}

        //entityCursor.step<uint64_t>();
        //for (size_t i = 0; i < mounsCount; ++i)
        //{
        //    *(spriteReadDescriptors + playerCount + skullCount + i) = { .start = entityCursor.getOffset() };

        //    entityCursor.step<Mouns>();
        //}

        //spriteEntitySection->readDescriptors = spriteReadDescriptors;
        //spriteEntitySection->descriptorCount = spriteCount;

        //entityCursor.step<uint64_t>();
        //for (size_t i = 0; i < obstacleCount; ++i)
        //{
        //    *(tiledSpriteReadDescriptors + i) = { .start = entityCursor.getOffset() };

        //    entityCursor.step<Obstacle>();
        //}

        //entityCursor.step<uint64_t>();
        //for (size_t i = 0; i < movingPlatformCount; ++i)
        //{
        //    *(tiledSpriteReadDescriptors + obstacleCount + i) = { .start = entityCursor.getOffset() };

        //    entityCursor.step<MovingPlatform>();
        //}

        //tiledSpriteEntitySection->readDescriptors = tiledSpriteReadDescriptors;
        //tiledSpriteEntitySection->descriptorCount = tiledSpriteCount;

        //entityCursor.step<uint64_t>();
        //for (size_t i = 0; i < colliderCount; ++i)
        //{
        //    *(colliderReadDescriptors + i) = { .start = entityCursor.getOffset() };

        //    entityCursor.step<Collider2DEntity>();
        //}

        //colliderEntitySection->readDescriptors = colliderReadDescriptors;
        //colliderEntitySection->descriptorCount = colliderCount;

        return true;
    }

    void updateGame(GameAllocator* allocator, double deltaTime)
    {
        constexpr double fixedDeltaTime{ 1.0 / 90 };

        auto* allocatorStart = reinterpret_cast<unsigned char*>(allocator->peek());

        const GameMemoryView gameMemory = readGameMemory(allocatorStart);
        double physicsAccumulator{ 0.0f };

        step(
            readLiveMemory(allocatorStart + getReservedByteSize(gameMemory)),
            gameMemory.input.data(),
            gameMemory.animations.data(),
            deltaTime,
            fixedDeltaTime,
            &physicsAccumulator);
    }

    RenderDescriptor initializeRenderDescriptor(const GameAllocator* allocator)
    {
        return { };

        /*GameMemory gameMemory = readGameMemory(allocator);
        
        const size_t entitySectionCount = *( gameMemory.entitySectionCount.data() );

        if (entitySectionCount == 0)
        {
            return { };
        }

        return {
            .entitySections     = reinterpret_cast<EntitySection*>(gameMemory.entitySectionSpan.data()),
            .entitySectionCount = entitySectionCount
        };*/
    }
}