#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

void SDL_Init_Driver();

class Sound
{
    Mix_Chunk* m_pChunk;
    Mix_Music* m_pMusic;

    int m_channel = -1;

public:
    const unsigned char* Buffer;
    size_t BufferSize;
    std::function<void()> PlayCallback;
    std::function<void()> StopCallback;
    bool IsPlaying;

    Sound(const unsigned char* buffer, size_t bufferSize)
    {
        Buffer = buffer;
        BufferSize = bufferSize;

        if (Buffer == nullptr || BufferSize <= 0 || BufferSize < 4)
        {
            printf("[*] Failed to create sound: audio buffer is empty.\n");
            return;
        }

        if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096) < 0)
        {
            printf("[*] Failed to open audio device: %s\n", Mix_GetError());
            return;
        }

        auto rw = SDL_RWFromConstMem(Buffer, BufferSize);

        if (!rw)
        {
            printf("[*] Failed to read audio buffer: %s\n", SDL_GetError());
            return;
        }

        // Check signature for 'RIFF' and load as a chunk.
        if (Buffer[0] == 'R' && Buffer[1] == 'I' && Buffer[2] == 'F' && Buffer[3] == 'F')
        {
            m_pChunk = Mix_LoadWAV_RW(rw, 1);
        }
        else
        {
            m_pMusic = Mix_LoadMUS_RW(rw, 1);
        }

        if (!m_pChunk && !m_pMusic)
            printf("[*] Failed to create sound: %s\n", Mix_GetError());
    }

    void Play()
    {
        if (m_pChunk)
            m_channel = Mix_PlayChannel(m_channel, m_pChunk, 0);

        if (m_pMusic)
            Mix_PlayMusic(m_pMusic, 0);

        IsPlaying = true;

        if (PlayCallback)
            PlayCallback();
    }

    void Pause()
    {
        if (m_pChunk)
            Mix_Pause(m_channel);

        if (m_pMusic)
            Mix_PauseMusic();

        IsPlaying = false;

        if (StopCallback)
            StopCallback();
    }
};
