#include <apu/audio.h>
#include <apu/embedded_player.h>
#include <user/config.h>

#include <res/music/installer.ogg.h>
#include <res/sounds/sys_worldmap_cursor.ogg.h>
#include <res/sounds/sys_worldmap_finaldecide.ogg.h>
#include <res/sounds/sys_actstg_pausecansel.ogg.h>
#include <res/sounds/sys_actstg_pausecursor.ogg.h>
#include <res/sounds/sys_actstg_pausedecide.ogg.h>
#include <res/sounds/sys_actstg_pausewinclose.ogg.h>
#include <res/sounds/sys_actstg_pausewinopen.ogg.h>

#include <res/melpontro/sounds/sndAmongUs.ogg.h>
#include <res/melpontro/sounds/sndBadApple.ogg.h>
#include <res/melpontro/sounds/sndJarate.ogg.h>
#include <res/melpontro/sounds/sndMystery.ogg.h>
#include <res/melpontro/sounds/sndPush.ogg.h>
#include <res/melpontro/sounds/sndSans.ogg.h>
#include <res/melpontro/sounds/sndVineBoom.ogg.h>
#include <res/melpontro/sounds/sndXboxNotify.ogg.h>

enum class EmbeddedSound
{
    SysWorldMapCursor,
    SysWorldMapFinalDecide,
    SysActStgPauseCansel,
    SysActStgPauseCursor,
    SysActStgPauseDecide,
    SysActStgPauseWinClose,
    SysActStgPauseWinOpen,
    AmongUs,
    BadApple,
    Jarate,
    Mystery,
    Push,
    Sans,
    VineBoom,
    XboxNotify,
    Count
};

struct EmbeddedSoundData
{
    Mix_Chunk* chunk{};
};

static std::array<EmbeddedSoundData, size_t(EmbeddedSound::Count)> g_embeddedSoundData = {};

static const std::unordered_map<std::string_view, EmbeddedSound> g_embeddedSoundMap =
{
    { "sys_worldmap_cursor", EmbeddedSound::SysWorldMapCursor },
    { "sys_worldmap_finaldecide", EmbeddedSound::SysWorldMapFinalDecide },
    { "sys_actstg_pausecansel", EmbeddedSound::SysActStgPauseCansel },
    { "sys_actstg_pausecursor", EmbeddedSound::SysActStgPauseCursor },
    { "sys_actstg_pausedecide", EmbeddedSound::SysActStgPauseDecide },
    { "sys_actstg_pausewinclose", EmbeddedSound::SysActStgPauseWinClose },
    { "sys_actstg_pausewinopen", EmbeddedSound::SysActStgPauseWinOpen },
    { "AmongUs", EmbeddedSound::AmongUs },
    { "BadApple", EmbeddedSound::BadApple },
    { "Jarate", EmbeddedSound::Jarate },
    { "Mystery", EmbeddedSound::Mystery },
    { "Push", EmbeddedSound::Push },
    { "Sans", EmbeddedSound::Sans },
    { "VineBoom", EmbeddedSound::VineBoom },
    { "XboxNotify", EmbeddedSound::XboxNotify }
};

static size_t g_channelIndex;

static void PlayEmbeddedSound(EmbeddedSound s)
{
    EmbeddedSoundData& data = g_embeddedSoundData[size_t(s)];

    if (data.chunk == nullptr)
    {
        // The sound hasn't been created yet, create it and pick it.
        const void* soundData = nullptr;
        size_t soundDataSize = 0;

        switch (s)
        {
            case EmbeddedSound::SysWorldMapCursor:
                soundData = g_sys_worldmap_cursor;
                soundDataSize = sizeof(g_sys_worldmap_cursor);
                break;
            case EmbeddedSound::SysWorldMapFinalDecide:
                soundData = g_sys_worldmap_finaldecide;
                soundDataSize = sizeof(g_sys_worldmap_finaldecide);
                break;
            case EmbeddedSound::SysActStgPauseCansel:
                soundData = g_sys_actstg_pausecansel;
                soundDataSize = sizeof(g_sys_actstg_pausecansel);
                break;
            case EmbeddedSound::SysActStgPauseCursor:
                soundData = g_sys_actstg_pausecursor;
                soundDataSize = sizeof(g_sys_actstg_pausecursor);
                break;
            case EmbeddedSound::SysActStgPauseDecide:
                soundData = g_sys_actstg_pausedecide;
                soundDataSize = sizeof(g_sys_actstg_pausedecide);
                break;
            case EmbeddedSound::SysActStgPauseWinClose:
                soundData = g_sys_actstg_pausewinclose;
                soundDataSize = sizeof(g_sys_actstg_pausewinclose);
                break;
            case EmbeddedSound::SysActStgPauseWinOpen:
                soundData = g_sys_actstg_pausewinopen;
                soundDataSize = sizeof(g_sys_actstg_pausewinopen);
                break;
            case EmbeddedSound::AmongUs:
                soundData = g_sndAmongUs;
                soundDataSize = sizeof(g_sndAmongUs);
                break;
            case EmbeddedSound::BadApple:
                soundData = g_sndBadApple;
                soundDataSize = sizeof(g_sndBadApple);
                break;
            case EmbeddedSound::Mystery:
                soundData = g_sndMystery;
                soundDataSize = sizeof(g_sndMystery);
                break;
            case EmbeddedSound::Push:
                soundData = g_sndPush;
                soundDataSize = sizeof(g_sndPush);
                break;
            case EmbeddedSound::Sans:
                soundData = g_sndSans;
                soundDataSize = sizeof(g_sndSans);
                break;
            case EmbeddedSound::Jarate:
                soundData = g_sndJarate;
                soundDataSize = sizeof(g_sndJarate);
                break;
            case EmbeddedSound::VineBoom:
                soundData = g_sndVineBoom;
                soundDataSize = sizeof(g_sndVineBoom);
                break;
            case EmbeddedSound::XboxNotify:
                soundData = g_sndXboxNotify;
                soundDataSize = sizeof(g_sndXboxNotify);
                break;
            default:
                assert(false && "Unknown embedded sound.");
                return;
        }

        data.chunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(soundData, soundDataSize), 1);
    }
    
    if (s == EmbeddedSound::BadApple)
    {
        Mix_VolumeChunk(data.chunk, 0.75f * MIX_MAX_VOLUME);
    }
    else
    {
        Mix_VolumeChunk(data.chunk, Config::MasterVolume * Config::EffectsVolume * MIX_MAX_VOLUME);
    }

    Mix_PlayChannel(g_channelIndex % MIX_CHANNELS, data.chunk, 0);

    ++g_channelIndex;
}

static Mix_Music* g_installerMusic;

void EmbeddedPlayer::Init() 
{
    if (s_isActive)
        return;

    Mix_OpenAudio(XAUDIO_SAMPLES_HZ, AUDIO_F32SYS, 2, 2048);
    g_installerMusic = Mix_LoadMUS_RW(SDL_RWFromConstMem(g_installer_music, sizeof(g_installer_music)), 1);

    s_isActive = true;
}

void EmbeddedPlayer::Play(const char* name) 
{
    assert(s_isActive && "Playback shouldn't be requested if the Embedded Player isn't active.");

    auto it = g_embeddedSoundMap.find(name);

    if (it == g_embeddedSoundMap.end())
        return;

    PlayEmbeddedSound(it->second);
}

void EmbeddedPlayer::PlayMusic()
{
    if (!Mix_PlayingMusic())
    {
        Mix_PlayMusic(g_installerMusic, INT_MAX);
        Mix_VolumeMusic(Config::MasterVolume * Config::MusicVolume * MUSIC_VOLUME * MIX_MAX_VOLUME);
    }
}

void EmbeddedPlayer::FadeOutMusic()
{
    if (Mix_PlayingMusic())
        Mix_FadeOutMusic(1000);
}

void EmbeddedPlayer::Shutdown() 
{
    for (EmbeddedSoundData& data : g_embeddedSoundData)
    {
        if (data.chunk != nullptr)
            Mix_FreeChunk(data.chunk);
    }

    Mix_HaltMusic();
    Mix_FreeMusic(g_installerMusic);

    Mix_CloseAudio();
    Mix_Quit();

    s_isActive = false;
}
