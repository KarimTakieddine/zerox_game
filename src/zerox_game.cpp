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
        backgrounds[0].entity.renderEntity.layer    = 1;
        backgrounds[0].scrollSpeed                  = { -0.0375f, 0.0f };

        configureTransform(backgrounds[1].entity.renderEntity.transform, { 0.0f, 0.0f }, { 33.0f, 16.875f });
        backgrounds[1].entity.renderEntity.layer    = 0;
        backgrounds[1].scrollSpeed                  = { 0.0625f, 0.0f };

            // -> Configure obstacle(s):

        constexpr size_t obstacleCount{ 4 };
        auto* obstacles = allocator->requestMemoryArray<Obstacle>(obstacleCount);

        configureTransform(obstacles[0].gameEntity.renderEntity.transform, { -15.25f, 0.0f }, { 1.0f, 18.0f });
        configureCollider(obstacles[0].collider, { 1.0f, 18.0f }, { 1.0f, 0.0f });
        configureMaterial(obstacles[0].gameEntity.renderEntity.material, { 0.0625f, 0.0625f }, { 0.125f, 0.9375f }, 1, 18);

        configureTransform(obstacles[1].gameEntity.renderEntity.transform, { 15.25f, 0.0f }, { 1.0f, 18.0f });
        configureCollider(obstacles[1].collider, { 1.0f, 18.0f }, { -1.0f, 0.0f });
        configureMaterial(obstacles[1].gameEntity.renderEntity.material, { 0.0625f, 0.0625f }, { 0.125f, 0.9375f }, 1, 18);

        configureTransform(obstacles[2].gameEntity.renderEntity.transform, { 0.0f, -8.0f }, { 32.0f, 1.0f });
        configureCollider(obstacles[2].collider, { 32.0f, 1.0f }, { 0.0f, 1.0f });
        configureMaterial(obstacles[2].gameEntity.renderEntity.material, { 0.0625f, 0.0625f }, { 0.0f, 0.9375f }, 16);

        configureTransform(obstacles[3].gameEntity.renderEntity.transform, { 0.0f, 8.0f }, { 32.0f, 1.0f });
        configureCollider(obstacles[3].collider, { 32.0f, 1.0f }, { 0.0f, -1.0f });
        configureMaterial(obstacles[3].gameEntity.renderEntity.material, { 0.0625f, 0.0625f }, { 0.0625f, 0.9375f }, 16);

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
        configureMaterial(skulls[3].gameEntity.renderEntity.material, { 0.0625f, 0.0625f }, { 0.0f, 0.1875f });

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
        const size_t reservedByteSize       = getReservedByteSize(gameMemoryView);

        allocator->free(allocator->getLiveSizeInBytes() - reservedByteSize);

        const auto levels = gameMemoryView.levels;
        if (index >= levels.size())
        {
            return false;
        }

        const auto* level = gameMemoryView.levels.data() + index;

        // Load live level:

        allocator->requestMemory<GameMemoryHeader>(GameMemoryHeader{ .type = MemoryType::LEVEL, .count = 1 });
        auto* liveLevel = allocator->requestMemoryArray<LevelView>(1);

        auto resetPhysics = [](const ZeroXBridge::Transform& transform, PhysicsState& state) -> void {
            state.currentPosition = getPosition(transform);
            state.lastPosition    = state.currentPosition;
        };

        const auto levelBackgrounds     = level->backgrounds;
        const size_t backgroundCount    = levelBackgrounds.size();

        auto* liveBackgrounds = allocator->requestMemoryArray<Background>(backgroundCount);
        for (size_t i = 0; i < backgroundCount; ++i)
        {
            Background* liveBackground  = liveBackgrounds + i;
            *(liveBackground)           = *(levelBackgrounds.data() + i);

            resetPhysics(liveBackground->entity.renderEntity.transform, liveBackground->entity.physicsState);
        }

        const auto levelObstacles   = level->obstacles;
        const size_t obstacleCount  = levelObstacles.size();

        auto* liveObstacles = allocator->requestMemoryArray<Obstacle>(obstacleCount);
        for (size_t i = 0; i < obstacleCount; ++i)
        {
            Obstacle* liveObstacle  = liveObstacles + i;
            *(liveObstacle)         = *(levelObstacles.data() + i);

            resetPhysics(liveObstacle->gameEntity.renderEntity.transform, liveObstacle->gameEntity.physicsState);
        }

        const auto levelMovingPlatforms     = level->movingPlatforms;
        const size_t movingPlatformCount    = levelMovingPlatforms.size();

        auto* liveMovingPlatforms = allocator->requestMemoryArray<MovingPlatform>(movingPlatformCount);
        for (size_t i = 0; i < movingPlatformCount; ++i)
        {
            MovingPlatform* liveMovingPlatform  = liveMovingPlatforms + i;
            *(liveMovingPlatform)               = *(levelMovingPlatforms.data() + i);

            resetPhysics(liveMovingPlatform->obstacle.gameEntity.renderEntity.transform, liveMovingPlatform->obstacle.gameEntity.physicsState);
        }

        const auto levelSkulls  = level->skulls;
        const size_t skullCount = levelSkulls.size();

        auto* liveSkulls = allocator->requestMemoryArray<Skull>(skullCount);
        for (size_t i = 0; i < skullCount; ++i)
        {
            Skull* liveSkull    = liveSkulls + i;
            *(liveSkull)        = *(levelSkulls.data() + i);

            resetPhysics(liveSkull->gameEntity.renderEntity.transform, liveSkull->gameEntity.physicsState);
        }

        const auto levelMouns   = level->mounsList;
        const size_t mounsCount = levelMouns.size();

        auto* liveMouns = allocator->requestMemoryArray<Mouns>(mounsCount);
        for (size_t i = 0; i < mounsCount; ++i)
        {
            Mouns* mouns    = liveMouns + i;
            *(mouns)        = *(levelMouns.data() + i);

            resetPhysics(mouns->gameEntity.renderEntity.transform, mouns->gameEntity.physicsState);
        }

        const auto levelPlayers     = level->players;
        const size_t playerCount    = levelPlayers.size();

        auto* livePlayers = allocator->requestMemoryArray<Player>(playerCount);

        liveLevel->backgrounds      = { liveBackgrounds, backgroundCount };
        liveLevel->obstacles        = { liveObstacles, obstacleCount };
        liveLevel->movingPlatforms  = { liveMovingPlatforms, movingPlatformCount };
        liveLevel->skulls           = { liveSkulls, skullCount };
        liveLevel->mounsList        = { liveMouns, mounsCount };
        liveLevel->players          = { livePlayers, playerCount };

        const size_t colliderCount = obstacleCount + movingPlatformCount + skullCount + mounsCount + playerCount;
        allocator->requestMemory<GameMemoryHeader>(GameMemoryHeader{ .type = MemoryType::COLLIDER_ENTITY, .count = static_cast<uint64_t>(colliderCount) });
        auto* colliderEntities = allocator->requestMemoryArray<Collider2DEntity>(colliderCount);

        // Load collider entities:

        for (size_t i = 0; i < obstacleCount; ++i)
        {
            Obstacle* obstacle = liveObstacles + i;

            colliderEntities->collider2D                                = &obstacle->collider;
            colliderEntities->target                                    = &obstacle->gameEntity.renderEntity.transform;
            colliderEntities->gameEntity.renderEntity.material.color    = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliderEntities;
        }

        for (size_t i = 0; i < movingPlatformCount; ++i)
        {
            MovingPlatform* movingPlatform = liveMovingPlatforms + i;

            colliderEntities->collider2D                                = &movingPlatform->obstacle.collider;
            colliderEntities->target                                    = &movingPlatform->obstacle.gameEntity.renderEntity.transform;
            colliderEntities->gameEntity.renderEntity.material.color    = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliderEntities;
        }

        for (size_t i = 0; i < skullCount; ++i)
        {
            Skull* skull = liveSkulls + i;

            colliderEntities->collider2D                                = &skull->collider;
            colliderEntities->target                                    = &skull->gameEntity.renderEntity.transform;
            colliderEntities->gameEntity.renderEntity.material.color    = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliderEntities;
        }

        for (size_t i = 0; i < mounsCount; ++i)
        {
            Mouns* mouns = liveMouns + i;

            colliderEntities->collider2D                                = &mouns->collider;
            colliderEntities->target                                    = &mouns->gameEntity.renderEntity.transform;
            colliderEntities->gameEntity.renderEntity.material.color    = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliderEntities;
        }

        for (size_t i = 0; i < playerCount; ++i)
        {
            Player* player = livePlayers + i;

            colliderEntities->collider2D                                = &player->collider;
            colliderEntities->target                                    = &player->gameEntity.renderEntity.transform;
            colliderEntities->gameEntity.renderEntity.material.color    = { 0.0f, 1.0f, 0.0f, 1.0f };

            ++colliderEntities;
        }

        // Load entity sections:

        const size_t entitySectionCount{ 4 };
        allocator->requestMemory<GameMemoryHeader>(GameMemoryHeader{ .type = MemoryType::ENTITY_SECTION, .count = static_cast<uint64_t>(entitySectionCount) });
        auto* entitySections = allocator->requestMemoryArray<ZeroXBridge::EntitySection>(entitySectionCount);

        size_t sectionOffset = reservedByteSize + sizeof(GameMemoryHeader) + sizeof(LevelView);

        auto* backgroundDescriptors = allocator->requestMemoryArray<ZeroXBridge::EntityReadDescriptor>(backgroundCount);
        for (size_t i = 0; i < backgroundCount; ++i)
        {
            (backgroundDescriptors + i)->start = sectionOffset;

            sectionOffset += sizeof(Background);
        }

        entitySections[0] = {
            .type               = EntitySectionType::BACKGROUND,
            .flags              = EntitySectionFlags::ALLOCATE_BATCH,
            .readDescriptors    = backgroundDescriptors,
            .descriptorCount    = backgroundCount
        };

        const size_t tileSpriteCount = obstacleCount + movingPlatformCount;
        auto* tiledSpriteDescriptors = allocator->requestMemoryArray<ZeroXBridge::EntityReadDescriptor>(tileSpriteCount);

        for (size_t i = 0; i < obstacleCount; ++i)
        {
            (tiledSpriteDescriptors + i)->start = sectionOffset;

            sectionOffset += sizeof(Obstacle);
        }

        for (size_t i = 0; i < movingPlatformCount; ++i)
        {
            (tiledSpriteDescriptors + obstacleCount + i)->start = sectionOffset;

            sectionOffset += sizeof(MovingPlatform);
        }

        entitySections[1] = {
            .type               = EntitySectionType::TILED_SPRITE,
            .flags              = EntitySectionFlags::ALLOCATE_BATCH,
            .readDescriptors    = tiledSpriteDescriptors,
            .descriptorCount    = tileSpriteCount
        };

        const size_t spriteCount    = skullCount + mounsCount + playerCount;
        auto* spriteDescriptors     = allocator->requestMemoryArray<ZeroXBridge::EntityReadDescriptor>(spriteCount);

        for (size_t i = 0; i < skullCount; ++i)
        {
            (spriteDescriptors + i)->start = sectionOffset;

            sectionOffset += sizeof(Skull);
        }

        for (size_t i = 0; i < mounsCount; ++i)
        {
            (spriteDescriptors + skullCount + i)->start = sectionOffset;

            sectionOffset += sizeof(Mouns);
        }

        for (size_t i = 0; i < playerCount; ++i)
        {
            (spriteDescriptors + mounsCount + skullCount + i)->start = sectionOffset;

            sectionOffset += sizeof(Player);
        }

        entitySections[2] = {
            .type               = EntitySectionType::SPRITE,
            .flags              = EntitySectionFlags::ALLOCATE_BATCH,
            .readDescriptors    = spriteDescriptors,
            .descriptorCount    = spriteCount
        };

        sectionOffset += sizeof(GameMemoryHeader);

        auto* colliderDescriptors = allocator->requestMemoryArray<ZeroXBridge::EntityReadDescriptor>(colliderCount);
        for (size_t i = 0; i < colliderCount; ++i)
        {
            (colliderDescriptors + i)->start = sectionOffset;

            sectionOffset += sizeof(Collider2DEntity);
        }

        entitySections[3] = {
            .type               = EntitySectionType::COLLIDER,
            .flags              = EntitySectionFlags::ALLOCATE_BATCH,
            .readDescriptors    = colliderDescriptors,
            .descriptorCount    = colliderCount
        };

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
        unsigned char* allocatorStart = reinterpret_cast<unsigned char*>(allocator->peek());

        const GameMemoryView gameMemory = readGameMemory(allocatorStart);
        const LiveMemoryView liveMemory = readLiveMemory(allocatorStart + getReservedByteSize(gameMemory));

        const auto entitySections = liveMemory.entitySections;

        return {
            .entitySections     = entitySections.data(),
            .entitySectionCount = static_cast<uint64_t>(entitySections.size())
        };
    }
}