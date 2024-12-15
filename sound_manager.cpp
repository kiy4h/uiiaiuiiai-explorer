#include "sound_manager.h"
#include <iostream>

SoundManager::SoundManager() : bgm(nullptr) {
    // Initialize SDL2 Mixer
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Failed to initialize SDL_Mixer: " << Mix_GetError() << std::endl;
    }
}

SoundManager::~SoundManager() {
    // Free loaded music
    if (bgm) {
        Mix_FreeMusic(bgm);
    }

    // Free all sound effects
    for (auto &pair : soundEffects) {
        Mix_FreeChunk(pair.second);
    }

    // Close SDL_Mixer
    Mix_CloseAudio();
    SDL_Quit();
}

bool SoundManager::loadBGM(const std::string &bgmPath) {
    bgm = Mix_LoadMUS(bgmPath.c_str());
    if (!bgm) {
        std::cerr << "Failed to load BGM: " << Mix_GetError() << std::endl;
        return false;
    }
    return true;
}

bool SoundManager::loadSoundEffect(const std::string &effectName, const std::string &effectPath) {
    Mix_Chunk *effect = Mix_LoadWAV(effectPath.c_str());
    if (!effect) {
        std::cerr << "Failed to load sound effect (" << effectName << "): " << Mix_GetError() << std::endl;
        return false;
    }
    soundEffects[effectName] = effect;
    return true;
}

void SoundManager::playBGM() {
    if (bgm) {
        Mix_PlayMusic(bgm, -1); // Loop indefinitely
    }
}

void SoundManager::stopBGM() {
    Mix_HaltMusic();
}

void SoundManager::playSoundEffect(const std::string &effectName) {
    auto it = soundEffects.find(effectName);
    if (it != soundEffects.end()) {
        Mix_PlayChannel(-1, it->second, 0); // Play on any free channel
    } else {
        std::cerr << "Sound effect not found: " << effectName << std::endl;
    }
}
